package com.alpine.app.data.discovery

import kotlinx.serialization.Serializable
import kotlinx.serialization.Transient

@Serializable
data class BridgeBeacon(
    val service: String,
    val protocolVersion: String,
    val restPort: Int,
    val bridgeVersion: String,
    val hostAddress: String,
    @Transient val lastSeen: Long = System.currentTimeMillis()
)
