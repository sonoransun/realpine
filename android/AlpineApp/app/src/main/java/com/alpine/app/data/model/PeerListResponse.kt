package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class PeerListResponse(
    val peers: List<PeerSummary>
)

@Serializable
data class PeerSummary(
    val peerId: Long,
    val ipAddress: String,
    val port: Int,
    val active: Boolean,
    val avgBandwidth: Long,
    val peakBandwidth: Long
)
