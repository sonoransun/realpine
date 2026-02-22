package com.alpine.app.data.broadcast

import com.google.gson.Gson
import com.google.gson.JsonObject
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.InetAddress
import java.util.UUID
import java.util.concurrent.ConcurrentHashMap

class BroadcastResponder(
    private val contentProvider: LocalContentProvider,
    private val fileServerPort: Int = 8091,
    private val listenPort: Int = 8090
) {
    private var listenSocket: DatagramSocket? = null
    private val responseSocket = DatagramSocket()
    private var listenJob: Job? = null
    private val gson = Gson()
    private val responderId = UUID.randomUUID().toString().take(8)
    private val queryTimestamps = ConcurrentHashMap<String, MutableList<Long>>()

    companion object {
        private const val RATE_LIMIT_WINDOW_MS = 10_000L
        private const val MAX_QUERIES_PER_WINDOW = 5
    }

    fun start(scope: CoroutineScope) {
        listenJob = scope.launch(Dispatchers.IO) {
            listenSocket = DatagramSocket(listenPort).apply { soTimeout = 1000 }
            val buffer = ByteArray(4096)
            while (isActive) {
                try {
                    val packet = DatagramPacket(buffer, buffer.size)
                    listenSocket?.receive(packet)
                    val message = String(packet.data, 0, packet.length)
                    launch { handleQuery(message) }
                } catch (_: java.net.SocketTimeoutException) {
                } catch (_: Exception) {
                    if (!isActive) break
                }
            }
        }
    }

    fun stop() {
        listenJob?.cancel()
        listenJob = null
        try { listenSocket?.close() } catch (_: Exception) {}
        listenSocket = null
    }

    private fun handleQuery(message: String) {
        try {
            val json = gson.fromJson(message, JsonObject::class.java)
            if (json.get("type")?.asString != "alpine_query") return

            val queryString = json.get("queryString")?.asString ?: return
            val senderAddress = json.get("senderAddress")?.asString ?: return

            // Rate limit per sender
            val now = System.currentTimeMillis()
            val timestamps = queryTimestamps.getOrPut(senderAddress) { mutableListOf() }
            synchronized(timestamps) {
                timestamps.removeAll { now - it > RATE_LIMIT_WINDOW_MS }
                if (timestamps.size >= MAX_QUERIES_PER_WINDOW) return
                timestamps.add(now)
            }
            val senderPort = json.get("senderPort")?.asInt ?: return
            val queryId = json.get("queryId")?.asLong ?: return

            val localAddress = NetworkUtils.getLocalIpAddress() ?: return
            if (senderAddress == localAddress) return

            val peerDescMax = json.get("peerDescMax")?.asInt ?: 50
            val results = contentProvider.search(queryString, localAddress, fileServerPort)
                .take(peerDescMax)
            if (results.isEmpty()) return

            val response = JsonObject().apply {
                addProperty("type", "alpine_response")
                addProperty("queryId", queryId)
                addProperty("responderId", responderId)
                add("resources", gson.toJsonTree(results))
            }

            val bytes = response.toString().toByteArray()
            responseSocket.send(DatagramPacket(
                bytes, bytes.size,
                InetAddress.getByName(senderAddress), senderPort
            ))
        } catch (_: Exception) {}
    }
}
