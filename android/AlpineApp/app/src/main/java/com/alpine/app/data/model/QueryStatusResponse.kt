package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class QueryStatusResponse(
    val inProgress: Boolean,
    val totalPeers: Long,
    val peersQueried: Long,
    val numPeerResponses: Long,
    val totalHits: Long
)
