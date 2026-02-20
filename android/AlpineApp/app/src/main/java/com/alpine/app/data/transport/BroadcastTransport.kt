package com.alpine.app.data.transport

import com.alpine.app.data.broadcast.NetworkUtils
import com.alpine.app.data.model.PeerResources
import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.model.QueryResponse
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.model.ResourceDesc
import com.google.gson.Gson
import com.google.gson.JsonObject
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.util.UUID
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicLong

class BroadcastTransport : QueryTransport {

    private val gson = Gson()
    private val senderId = UUID.randomUUID().toString().take(8)
    private val queryIdCounter = AtomicLong(System.currentTimeMillis())
    private val queries = ConcurrentHashMap<Long, QueryState>()
    private val supervisorJob = SupervisorJob()
    private val scope = CoroutineScope(supervisorJob + Dispatchers.IO)
    private val socket = DatagramSocket().apply {
        soTimeout = 1000
        broadcast = true
    }

    companion object {
        const val BROADCAST_PORT = 8090
        const val QUERY_TIMEOUT_MS = 30_000L
    }

    init { startReceiving() }

    private fun startReceiving() {
        scope.launch {
            val buffer = ByteArray(65535)
            while (isActive) {
                try {
                    val packet = DatagramPacket(buffer, buffer.size)
                    socket.receive(packet)
                    handleResponse(String(packet.data, 0, packet.length))
                } catch (_: java.net.SocketTimeoutException) {
                    checkTimeouts()
                } catch (_: Exception) {
                    if (!isActive) break
                }
            }
        }
    }

    private fun checkTimeouts() {
        val now = System.currentTimeMillis()
        for (state in queries.values) {
            if (state.inProgress && now - state.startTime > QUERY_TIMEOUT_MS) {
                state.inProgress = false
            }
        }
    }

    private fun handleResponse(message: String) {
        try {
            val json = gson.fromJson(message, JsonObject::class.java)
            if (json.get("type")?.asString != "alpine_response") return
            val queryId = json.get("queryId")?.asLong ?: return
            val state = queries[queryId] ?: return
            if (!state.inProgress) return

            val responderId = json.get("responderId")?.asString ?: return
            val resources = gson.fromJson(
                json.getAsJsonArray("resources"), Array<ResourceDesc>::class.java
            )?.toList() ?: return

            val peerId = responderId.hashCode().toLong() and 0x7FFFFFFFL
            state.results[responderId] = PeerResources(peerId, resources)

            if (state.results.values.sumOf { it.resources.size } >= state.request.autoHaltLimit) {
                state.inProgress = false
            }
        } catch (_: Exception) {}
    }

    private suspend fun sendBroadcast(json: JsonObject) {
        val bytes = json.toString().toByteArray()
        withContext(Dispatchers.IO) {
            socket.send(DatagramPacket(
                bytes, bytes.size,
                InetAddress.getByName("255.255.255.255"), BROADCAST_PORT
            ))
        }
    }

    override suspend fun startQuery(request: QueryRequest): Result<QueryResponse> {
        return try {
            val queryId = queryIdCounter.incrementAndGet()
            queries[queryId] = QueryState(request)

            sendBroadcast(JsonObject().apply {
                addProperty("type", "alpine_query")
                addProperty("queryId", queryId)
                addProperty("senderId", senderId)
                addProperty("senderAddress", NetworkUtils.getLocalIpAddress() ?: "0.0.0.0")
                addProperty("senderPort", socket.localPort)
                addProperty("queryString", request.queryString)
                addProperty("groupName", request.groupName)
                addProperty("autoHaltLimit", request.autoHaltLimit)
                addProperty("peerDescMax", request.peerDescMax)
                addProperty("timestamp", System.currentTimeMillis())
            })

            Result.success(QueryResponse(queryId))
        } catch (e: Exception) {
            Result.failure(e)
        }
    }

    override suspend fun getQueryStatus(queryId: Long): Result<QueryStatusResponse> {
        val state = queries[queryId]
            ?: return Result.failure(Exception("Query $queryId not found"))
        return Result.success(QueryStatusResponse(
            inProgress = state.inProgress,
            totalPeers = 0,
            peersQueried = 0,
            numPeerResponses = state.results.size.toLong(),
            totalHits = state.results.values.sumOf { it.resources.size.toLong() }
        ))
    }

    override suspend fun getQueryResults(queryId: Long): Result<QueryResultsResponse> {
        val state = queries[queryId]
            ?: return Result.failure(Exception("Query $queryId not found"))
        return Result.success(QueryResultsResponse(state.results.values.toList()))
    }

    override suspend fun cancelQuery(queryId: Long): Result<Unit> {
        queries[queryId]?.inProgress = false
        try {
            sendBroadcast(JsonObject().apply {
                addProperty("type", "alpine_cancel")
                addProperty("queryId", queryId)
                addProperty("senderId", senderId)
            })
        } catch (_: Exception) {}
        return Result.success(Unit)
    }

    override fun shutdown() {
        supervisorJob.cancel()
        try { socket.close() } catch (_: Exception) {}
        queries.clear()
    }

    private class QueryState(
        val request: QueryRequest,
        val startTime: Long = System.currentTimeMillis(),
        val results: ConcurrentHashMap<String, PeerResources> = ConcurrentHashMap(),
        @Volatile var inProgress: Boolean = true
    )
}
