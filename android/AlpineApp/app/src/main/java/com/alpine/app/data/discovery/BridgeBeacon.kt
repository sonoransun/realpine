package com.alpine.app.data.discovery

data class BridgeBeacon(
    val service: String,
    val protocolVersion: String,
    val restPort: Int,
    val bridgeVersion: String,
    val hostAddress: String,
    val lastSeen: Long = System.currentTimeMillis()
)
