package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class PeerDetail(
    val peerId: Long,
    val ipAddress: String,
    val port: Long,
    val lastRecvTime: Long,
    val lastSendTime: Long,
    val avgBandwidth: Long,
    val peakBandwidth: Long
)
