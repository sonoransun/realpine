package com.alpine.app.data.rpc

import com.alpine.app.data.model.FilterModels
import com.alpine.app.data.model.GroupInfo
import com.alpine.app.data.model.ModuleInfo
import com.alpine.app.data.model.PeerDetail
import com.alpine.app.data.model.PeerResources
import com.alpine.app.data.model.QueryResultsResponse
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.model.ResourceDesc
import com.alpine.app.data.model.ServerStatus
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.jsonArray
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.long
import kotlinx.serialization.json.boolean
import kotlinx.serialization.json.int
import kotlinx.serialization.json.put

class AlpineRpcService(
    baseUrl: String,
    tlsConfig: TlsConfig = TlsConfig(),
    host: String = "",
    apiKey: String = ""
) {
    private val client = JsonRpcClient(baseUrl, tlsConfig, host, apiKey)

    private fun JsonElement.safeAsJsonObject(): JsonObject =
        takeIf { it is JsonObject }?.jsonObject
            ?: throw JsonRpcException(-32603, "Invalid response format")

    // --- Status ---

    suspend fun getStatus(): ServerStatus {
        val result = client.call("getStatus").safeAsJsonObject()
        return ServerStatus(
            status = result["status"]?.jsonPrimitive?.content ?: "unknown",
            version = result["version"]?.jsonPrimitive?.content ?: "unknown"
        )
    }

    // --- Query ---

    suspend fun startQuery(
        queryString: String,
        groupName: String = "",
        autoHaltLimit: Long = 100,
        peerDescMax: Long = 50
    ): Long {
        val params = buildJsonObject {
            put("queryString", queryString)
            if (groupName.isNotBlank()) put("groupName", groupName)
            put("autoHaltLimit", autoHaltLimit)
            put("peerDescMax", peerDescMax)
        }
        val result = client.call("startQuery", params).safeAsJsonObject()
        return result["queryId"]!!.jsonPrimitive.long
    }

    suspend fun queryInProgress(queryId: Long): Boolean {
        val params = buildJsonObject { put("queryId", queryId) }
        val result = client.call("queryInProgress", params).safeAsJsonObject()
        return result["inProgress"]!!.jsonPrimitive.boolean
    }

    suspend fun getQueryStatus(queryId: Long): QueryStatusResponse {
        val params = buildJsonObject { put("queryId", queryId) }
        val result = client.call("getQueryStatus", params).safeAsJsonObject()
        return QueryStatusResponse(
            inProgress = true,
            totalPeers = result["totalPeers"]?.jsonPrimitive?.long ?: 0,
            peersQueried = result["peersQueried"]?.jsonPrimitive?.long ?: 0,
            numPeerResponses = result["numPeerResponses"]?.jsonPrimitive?.long ?: 0,
            totalHits = result["totalHits"]?.jsonPrimitive?.long ?: 0
        )
    }

    suspend fun getQueryResults(queryId: Long): QueryResultsResponse {
        val params = buildJsonObject { put("queryId", queryId) }
        val result = client.call("getQueryResults", params).safeAsJsonObject()
        val peers = result["peers"]?.jsonArray?.map { peerEl ->
            val peer = peerEl.jsonObject
            val resources = peer["resources"]?.jsonArray?.map { resEl ->
                val res = resEl.jsonObject
                val locators = res["locators"]?.jsonArray?.map { it.jsonPrimitive.content } ?: emptyList()
                ResourceDesc(
                    resourceId = res["resourceId"]?.jsonPrimitive?.long ?: 0,
                    size = res["size"]?.jsonPrimitive?.long ?: 0,
                    description = res["description"]?.jsonPrimitive?.content ?: "",
                    locators = locators
                )
            } ?: emptyList()
            PeerResources(
                peerId = peer["peerId"]?.jsonPrimitive?.long ?: 0,
                resources = resources
            )
        } ?: emptyList()
        return QueryResultsResponse(peers)
    }

    suspend fun pauseQuery(queryId: Long): Boolean {
        val params = buildJsonObject { put("queryId", queryId) }
        val result = client.call("pauseQuery", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun resumeQuery(queryId: Long): Boolean {
        val params = buildJsonObject { put("queryId", queryId) }
        val result = client.call("resumeQuery", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun cancelQuery(queryId: Long): Boolean {
        val params = buildJsonObject { put("queryId", queryId) }
        val result = client.call("cancelQuery", params).safeAsJsonObject()
        return result["cancelled"]?.jsonPrimitive?.boolean ?: false
    }

    // --- Peers ---

    suspend fun getAllPeers(): List<Long> {
        val result = client.call("getAllPeers").safeAsJsonObject()
        return result["peerIds"]?.jsonArray?.map { it.jsonPrimitive.long } ?: emptyList()
    }

    suspend fun getPeer(peerId: Long): PeerDetail {
        val params = buildJsonObject { put("peerId", peerId) }
        val result = client.call("getPeer", params).safeAsJsonObject()
        return PeerDetail(
            peerId = result["peerId"]?.jsonPrimitive?.long ?: peerId,
            ipAddress = result["ipAddress"]?.jsonPrimitive?.content ?: "",
            port = result["port"]?.jsonPrimitive?.long ?: 0,
            lastRecvTime = result["lastRecvTime"]?.jsonPrimitive?.long ?: 0,
            lastSendTime = result["lastSendTime"]?.jsonPrimitive?.long ?: 0,
            avgBandwidth = result["avgBandwidth"]?.jsonPrimitive?.long ?: 0,
            peakBandwidth = result["peakBandwidth"]?.jsonPrimitive?.long ?: 0
        )
    }

    suspend fun addPeer(ipAddress: String, port: Long): Boolean {
        val params = buildJsonObject {
            put("ipAddress", ipAddress)
            put("port", port)
        }
        val result = client.call("addPeer", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun getPeerId(ipAddress: String, port: Long): Long {
        val params = buildJsonObject {
            put("ipAddress", ipAddress)
            put("port", port)
        }
        val result = client.call("getPeerId", params).safeAsJsonObject()
        return result["peerId"]?.jsonPrimitive?.long ?: 0
    }

    suspend fun activatePeer(peerId: Long): Boolean {
        val params = buildJsonObject { put("peerId", peerId) }
        val result = client.call("activatePeer", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun deactivatePeer(peerId: Long): Boolean {
        val params = buildJsonObject { put("peerId", peerId) }
        val result = client.call("deactivatePeer", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun pingPeer(peerId: Long): Boolean {
        val params = buildJsonObject { put("peerId", peerId) }
        val result = client.call("pingPeer", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    // --- Network Filters ---

    suspend fun excludeHost(ipAddress: String): Boolean {
        val params = buildJsonObject { put("ipAddress", ipAddress) }
        val result = client.call("excludeHost", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun excludeSubnet(subnetIpAddress: String, subnetMask: String): Boolean {
        val params = buildJsonObject {
            put("subnetIpAddress", subnetIpAddress)
            put("subnetMask", subnetMask)
        }
        val result = client.call("excludeSubnet", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun allowHost(ipAddress: String): Boolean {
        val params = buildJsonObject { put("ipAddress", ipAddress) }
        val result = client.call("allowHost", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun allowSubnet(subnetIpAddress: String, subnetMask: String): Boolean {
        val params = buildJsonObject {
            put("subnetIpAddress", subnetIpAddress)
            put("subnetMask", subnetMask)
        }
        val result = client.call("allowSubnet", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun listExcludedHosts(): List<String> {
        val result = client.call("listExcludedHosts").safeAsJsonObject()
        return result["hosts"]?.jsonArray?.map { it.jsonPrimitive.content } ?: emptyList()
    }

    suspend fun listExcludedSubnets(): List<FilterModels.Subnet> {
        val result = client.call("listExcludedSubnets").safeAsJsonObject()
        return result["subnets"]?.jsonArray?.map { el ->
            val obj = el.jsonObject
            FilterModels.Subnet(
                ipAddress = obj["ipAddress"]?.jsonPrimitive?.content ?: "",
                netMask = obj["netMask"]?.jsonPrimitive?.content ?: ""
            )
        } ?: emptyList()
    }

    // --- Groups ---

    suspend fun createGroup(name: String, description: String = ""): Long {
        val params = buildJsonObject {
            put("name", name)
            if (description.isNotBlank()) put("description", description)
        }
        val result = client.call("createGroup", params).safeAsJsonObject()
        return result["groupId"]!!.jsonPrimitive.long
    }

    suspend fun deleteGroup(groupId: Long): Boolean {
        val params = buildJsonObject { put("groupId", groupId) }
        val result = client.call("deleteGroup", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun listGroups(): List<Long> {
        val result = client.call("listGroups").safeAsJsonObject()
        return result["groupIds"]?.jsonArray?.map { it.jsonPrimitive.long } ?: emptyList()
    }

    suspend fun getGroupInfo(groupId: Long): GroupInfo {
        val params = buildJsonObject { put("groupId", groupId) }
        return parseGroupInfo(client.call("getGroupInfo", params).safeAsJsonObject())
    }

    suspend fun getDefaultGroupInfo(): GroupInfo {
        return parseGroupInfo(client.call("getDefaultGroupInfo").safeAsJsonObject())
    }

    suspend fun getGroupPeerList(groupId: Long): List<Long> {
        val params = buildJsonObject { put("groupId", groupId) }
        val result = client.call("getGroupPeerList", params).safeAsJsonObject()
        return result["peerIds"]?.jsonArray?.map { it.jsonPrimitive.long } ?: emptyList()
    }

    suspend fun addPeerToGroup(groupId: Long, peerId: Long): Boolean {
        val params = buildJsonObject {
            put("groupId", groupId)
            put("peerId", peerId)
        }
        val result = client.call("addPeerToGroup", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun removePeerFromGroup(groupId: Long, peerId: Long): Boolean {
        val params = buildJsonObject {
            put("groupId", groupId)
            put("peerId", peerId)
        }
        val result = client.call("removePeerFromGroup", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    // --- Modules ---

    suspend fun registerModule(libraryPath: String, bootstrapSymbol: String): Long {
        val params = buildJsonObject {
            put("libraryPath", libraryPath)
            put("bootstrapSymbol", bootstrapSymbol)
        }
        val result = client.call("registerModule", params).safeAsJsonObject()
        return result["moduleId"]!!.jsonPrimitive.long
    }

    suspend fun unregisterModule(moduleId: Long): Boolean {
        val params = buildJsonObject { put("moduleId", moduleId) }
        val result = client.call("unregisterModule", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun loadModule(moduleId: Long): Boolean {
        val params = buildJsonObject { put("moduleId", moduleId) }
        val result = client.call("loadModule", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun unloadModule(moduleId: Long): Boolean {
        val params = buildJsonObject { put("moduleId", moduleId) }
        val result = client.call("unloadModule", params).safeAsJsonObject()
        return result["success"]?.jsonPrimitive?.boolean ?: false
    }

    suspend fun listActiveModules(): List<Long> {
        val result = client.call("listActiveModules").safeAsJsonObject()
        return result["moduleIds"]?.jsonArray?.map { it.jsonPrimitive.long } ?: emptyList()
    }

    suspend fun listAllModules(): List<Long> {
        val result = client.call("listAllModules").safeAsJsonObject()
        return result["moduleIds"]?.jsonArray?.map { it.jsonPrimitive.long } ?: emptyList()
    }

    suspend fun getModuleInfo(moduleId: Long): ModuleInfo {
        val params = buildJsonObject { put("moduleId", moduleId) }
        val result = client.call("getModuleInfo", params).safeAsJsonObject()
        return ModuleInfo(
            moduleId = result["moduleId"]?.jsonPrimitive?.long ?: moduleId,
            moduleName = result["moduleName"]?.jsonPrimitive?.content ?: "",
            description = result["description"]?.jsonPrimitive?.content ?: "",
            version = result["version"]?.jsonPrimitive?.content ?: "",
            libraryPath = result["libraryPath"]?.jsonPrimitive?.content ?: "",
            bootstrapSymbol = result["bootstrapSymbol"]?.jsonPrimitive?.content ?: "",
            activeTime = result["activeTime"]?.jsonPrimitive?.long ?: 0
        )
    }

    fun shutdown() {
        client.shutdown()
    }

    private fun parseGroupInfo(obj: JsonObject): GroupInfo {
        return GroupInfo(
            groupId = obj["groupId"]?.jsonPrimitive?.long ?: 0,
            groupName = obj["groupName"]?.jsonPrimitive?.content ?: "",
            description = obj["description"]?.jsonPrimitive?.content ?: "",
            numPeers = obj["numPeers"]?.jsonPrimitive?.long ?: 0,
            totalQueries = obj["totalQueries"]?.jsonPrimitive?.long ?: 0,
            totalResponses = obj["totalResponses"]?.jsonPrimitive?.long ?: 0
        )
    }
}
