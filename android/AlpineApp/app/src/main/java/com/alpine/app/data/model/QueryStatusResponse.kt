package com.alpine.app.data.model

data class QueryStatusResponse(
    val inProgress: Boolean,
    val totalPeers: Long,
    val peersQueried: Long,
    val numPeerResponses: Long,
    val totalHits: Long
)
