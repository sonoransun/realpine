package com.alpine.app.data.model

data class QueryResultsResponse(
    val peers: List<PeerResources>
)

data class PeerResources(
    val peerId: Long,
    val resources: List<ResourceDesc>
)

data class ResourceDesc(
    val resourceId: Long,
    val size: Long,
    val locators: List<String>,
    val description: String
)
