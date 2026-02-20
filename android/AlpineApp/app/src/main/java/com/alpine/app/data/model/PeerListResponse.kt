package com.alpine.app.data.model

data class PeerListResponse(
    val peers: List<PeerSummary>
)

data class PeerSummary(
    val peerId: Long,
    val ipAddress: String,
    val port: Int,
    val active: Boolean,
    val avgBandwidth: Long,
    val peakBandwidth: Long
)
