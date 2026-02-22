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
import com.google.gson.JsonArray
import com.google.gson.JsonObject

class AlpineRpcService(
    baseUrl: String,
    tlsConfig: TlsConfig = TlsConfig(),
    host: String = ""
) {
    private val client = JsonRpcClient(baseUrl, tlsConfig, host)

    // --- Status ---

    suspend fun getStatus(): ServerStatus {
        val result = client.call("getStatus").asJsonObject
        return ServerStatus(
            status = result.get("status")?.asString ?: "unknown",
            version = result.get("version")?.asString ?: "unknown"
        )
    }

    // --- Query ---

    suspend fun startQuery(
        queryString: String,
        groupName: String = "",
        autoHaltLimit: Long = 100,
        peerDescMax: Long = 50
    ): Long {
        val params = JsonObject().apply {
            addProperty("queryString", queryString)
            if (groupName.isNotBlank()) addProperty("groupName", groupName)
            addProperty("autoHaltLimit", autoHaltLimit)
            addProperty("peerDescMax", peerDescMax)
        }
        val result = client.call("startQuery", params).asJsonObject
        return result.get("queryId").asLong
    }

    suspend fun queryInProgress(queryId: Long): Boolean {
        val params = JsonObject().apply { addProperty("queryId", queryId) }
        val result = client.call("queryInProgress", params).asJsonObject
        return result.get("inProgress").asBoolean
    }

    suspend fun getQueryStatus(queryId: Long): QueryStatusResponse {
        val params = JsonObject().apply { addProperty("queryId", queryId) }
        val result = client.call("getQueryStatus", params).asJsonObject
        return QueryStatusResponse(
            inProgress = true,
            totalPeers = result.get("totalPeers")?.asLong ?: 0,
            peersQueried = result.get("peersQueried")?.asLong ?: 0,
            numPeerResponses = result.get("numPeerResponses")?.asLong ?: 0,
            totalHits = result.get("totalHits")?.asLong ?: 0
        )
    }

    suspend fun getQueryResults(queryId: Long): QueryResultsResponse {
        val params = JsonObject().apply { addProperty("queryId", queryId) }
        val result = client.call("getQueryResults", params).asJsonObject
        val peers = result.getAsJsonArray("peers")?.map { peerEl ->
            val peer = peerEl.asJsonObject
            val resources = peer.getAsJsonArray("resources")?.map { resEl ->
                val res = resEl.asJsonObject
                val locators = res.getAsJsonArray("locators")?.map { it.asString } ?: emptyList()
                ResourceDesc(
                    resourceId = res.get("resourceId")?.asLong ?: 0,
                    size = res.get("size")?.asLong ?: 0,
                    description = res.get("description")?.asString ?: "",
                    locators = locators
                )
            } ?: emptyList()
            PeerResources(
                peerId = peer.get("peerId")?.asLong ?: 0,
                resources = resources
            )
        } ?: emptyList()
        return QueryResultsResponse(peers)
    }

    suspend fun pauseQuery(queryId: Long): Boolean {
        val params = JsonObject().apply { addProperty("queryId", queryId) }
        val result = client.call("pauseQuery", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun resumeQuery(queryId: Long): Boolean {
        val params = JsonObject().apply { addProperty("queryId", queryId) }
        val result = client.call("resumeQuery", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun cancelQuery(queryId: Long): Boolean {
        val params = JsonObject().apply { addProperty("queryId", queryId) }
        val result = client.call("cancelQuery", params).asJsonObject
        return result.get("cancelled")?.asBoolean ?: false
    }

    // --- Peers ---

    suspend fun getAllPeers(): List<Long> {
        val result = client.call("getAllPeers").asJsonObject
        return result.getAsJsonArray("peerIds")?.map { it.asLong } ?: emptyList()
    }

    suspend fun getPeer(peerId: Long): PeerDetail {
        val params = JsonObject().apply { addProperty("peerId", peerId) }
        val result = client.call("getPeer", params).asJsonObject
        return PeerDetail(
            peerId = result.get("peerId")?.asLong ?: peerId,
            ipAddress = result.get("ipAddress")?.asString ?: "",
            port = result.get("port")?.asLong ?: 0,
            lastRecvTime = result.get("lastRecvTime")?.asLong ?: 0,
            lastSendTime = result.get("lastSendTime")?.asLong ?: 0,
            avgBandwidth = result.get("avgBandwidth")?.asLong ?: 0,
            peakBandwidth = result.get("peakBandwidth")?.asLong ?: 0
        )
    }

    suspend fun addPeer(ipAddress: String, port: Long): Boolean {
        val params = JsonObject().apply {
            addProperty("ipAddress", ipAddress)
            addProperty("port", port)
        }
        val result = client.call("addPeer", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun getPeerId(ipAddress: String, port: Long): Long {
        val params = JsonObject().apply {
            addProperty("ipAddress", ipAddress)
            addProperty("port", port)
        }
        val result = client.call("getPeerId", params).asJsonObject
        return result.get("peerId")?.asLong ?: 0
    }

    suspend fun activatePeer(peerId: Long): Boolean {
        val params = JsonObject().apply { addProperty("peerId", peerId) }
        val result = client.call("activatePeer", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun deactivatePeer(peerId: Long): Boolean {
        val params = JsonObject().apply { addProperty("peerId", peerId) }
        val result = client.call("deactivatePeer", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun pingPeer(peerId: Long): Boolean {
        val params = JsonObject().apply { addProperty("peerId", peerId) }
        val result = client.call("pingPeer", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    // --- Network Filters ---

    suspend fun excludeHost(ipAddress: String): Boolean {
        val params = JsonObject().apply { addProperty("ipAddress", ipAddress) }
        val result = client.call("excludeHost", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun excludeSubnet(subnetIpAddress: String, subnetMask: String): Boolean {
        val params = JsonObject().apply {
            addProperty("subnetIpAddress", subnetIpAddress)
            addProperty("subnetMask", subnetMask)
        }
        val result = client.call("excludeSubnet", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun allowHost(ipAddress: String): Boolean {
        val params = JsonObject().apply { addProperty("ipAddress", ipAddress) }
        val result = client.call("allowHost", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun allowSubnet(subnetIpAddress: String, subnetMask: String): Boolean {
        val params = JsonObject().apply {
            addProperty("subnetIpAddress", subnetIpAddress)
            addProperty("subnetMask", subnetMask)
        }
        val result = client.call("allowSubnet", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun listExcludedHosts(): List<String> {
        val result = client.call("listExcludedHosts").asJsonObject
        return result.getAsJsonArray("hosts")?.map { it.asString } ?: emptyList()
    }

    suspend fun listExcludedSubnets(): List<FilterModels.Subnet> {
        val result = client.call("listExcludedSubnets").asJsonObject
        return result.getAsJsonArray("subnets")?.map { el ->
            val obj = el.asJsonObject
            FilterModels.Subnet(
                ipAddress = obj.get("ipAddress")?.asString ?: "",
                netMask = obj.get("netMask")?.asString ?: ""
            )
        } ?: emptyList()
    }

    // --- Groups ---

    suspend fun createGroup(name: String, description: String = ""): Long {
        val params = JsonObject().apply {
            addProperty("name", name)
            if (description.isNotBlank()) addProperty("description", description)
        }
        val result = client.call("createGroup", params).asJsonObject
        return result.get("groupId").asLong
    }

    suspend fun deleteGroup(groupId: Long): Boolean {
        val params = JsonObject().apply { addProperty("groupId", groupId) }
        val result = client.call("deleteGroup", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun listGroups(): List<Long> {
        val result = client.call("listGroups").asJsonObject
        return result.getAsJsonArray("groupIds")?.map { it.asLong } ?: emptyList()
    }

    suspend fun getGroupInfo(groupId: Long): GroupInfo {
        val params = JsonObject().apply { addProperty("groupId", groupId) }
        return parseGroupInfo(client.call("getGroupInfo", params).asJsonObject)
    }

    suspend fun getDefaultGroupInfo(): GroupInfo {
        return parseGroupInfo(client.call("getDefaultGroupInfo").asJsonObject)
    }

    suspend fun getGroupPeerList(groupId: Long): List<Long> {
        val params = JsonObject().apply { addProperty("groupId", groupId) }
        val result = client.call("getGroupPeerList", params).asJsonObject
        return result.getAsJsonArray("peerIds")?.map { it.asLong } ?: emptyList()
    }

    suspend fun addPeerToGroup(groupId: Long, peerId: Long): Boolean {
        val params = JsonObject().apply {
            addProperty("groupId", groupId)
            addProperty("peerId", peerId)
        }
        val result = client.call("addPeerToGroup", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun removePeerFromGroup(groupId: Long, peerId: Long): Boolean {
        val params = JsonObject().apply {
            addProperty("groupId", groupId)
            addProperty("peerId", peerId)
        }
        val result = client.call("removePeerFromGroup", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    // --- Modules ---

    suspend fun registerModule(libraryPath: String, bootstrapSymbol: String): Long {
        val params = JsonObject().apply {
            addProperty("libraryPath", libraryPath)
            addProperty("bootstrapSymbol", bootstrapSymbol)
        }
        val result = client.call("registerModule", params).asJsonObject
        return result.get("moduleId").asLong
    }

    suspend fun unregisterModule(moduleId: Long): Boolean {
        val params = JsonObject().apply { addProperty("moduleId", moduleId) }
        val result = client.call("unregisterModule", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun loadModule(moduleId: Long): Boolean {
        val params = JsonObject().apply { addProperty("moduleId", moduleId) }
        val result = client.call("loadModule", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun unloadModule(moduleId: Long): Boolean {
        val params = JsonObject().apply { addProperty("moduleId", moduleId) }
        val result = client.call("unloadModule", params).asJsonObject
        return result.get("success")?.asBoolean ?: false
    }

    suspend fun listActiveModules(): List<Long> {
        val result = client.call("listActiveModules").asJsonObject
        return result.getAsJsonArray("moduleIds")?.map { it.asLong } ?: emptyList()
    }

    suspend fun listAllModules(): List<Long> {
        val result = client.call("listAllModules").asJsonObject
        return result.getAsJsonArray("moduleIds")?.map { it.asLong } ?: emptyList()
    }

    suspend fun getModuleInfo(moduleId: Long): ModuleInfo {
        val params = JsonObject().apply { addProperty("moduleId", moduleId) }
        val result = client.call("getModuleInfo", params).asJsonObject
        return ModuleInfo(
            moduleId = result.get("moduleId")?.asLong ?: moduleId,
            moduleName = result.get("moduleName")?.asString ?: "",
            description = result.get("description")?.asString ?: "",
            version = result.get("version")?.asString ?: "",
            libraryPath = result.get("libraryPath")?.asString ?: "",
            bootstrapSymbol = result.get("bootstrapSymbol")?.asString ?: "",
            activeTime = result.get("activeTime")?.asLong ?: 0
        )
    }

    fun shutdown() {
        client.shutdown()
    }

    private fun parseGroupInfo(obj: JsonObject): GroupInfo {
        return GroupInfo(
            groupId = obj.get("groupId")?.asLong ?: 0,
            groupName = obj.get("groupName")?.asString ?: "",
            description = obj.get("description")?.asString ?: "",
            numPeers = obj.get("numPeers")?.asLong ?: 0,
            totalQueries = obj.get("totalQueries")?.asLong ?: 0,
            totalResponses = obj.get("totalResponses")?.asLong ?: 0
        )
    }
}
