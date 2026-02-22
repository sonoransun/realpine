package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class QueryResultsResponse(
    val peers: List<PeerResources>
)

@Serializable
data class PeerResources(
    val peerId: Long,
    val resources: List<ResourceDesc>
)

@Serializable
data class ResourceDesc(
    val resourceId: Long,
    val size: Long,
    val locators: List<String>,
    val description: String
)
