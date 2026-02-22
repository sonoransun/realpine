package com.alpine.app.data.discovery

import android.content.Context
import android.net.wifi.WifiManager
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.int
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.SocketTimeoutException

class WifiDiscoveryManager(private val context: Context) {

    companion object {
        const val DEFAULT_BEACON_PORT = 8089
        private const val SOCKET_TIMEOUT_MS = 4000
        private const val STALE_THRESHOLD_MS = 10_000L
        private const val CLEANUP_INTERVAL_MS = 3_000L
        private const val RECEIVE_BUFFER_SIZE = 1024
    }

    private val json = Json { ignoreUnknownKeys = true }
    private val beacons = mutableMapOf<String, BridgeBeacon>()

    private val _discoveredBridges = MutableStateFlow<List<BridgeBeacon>>(emptyList())
    val discoveredBridges: StateFlow<List<BridgeBeacon>> = _discoveredBridges.asStateFlow()

    private val _isScanning = MutableStateFlow(false)
    val isScanning: StateFlow<Boolean> = _isScanning.asStateFlow()

    private var listenJob: Job? = null
    private var cleanupJob: Job? = null
    private var socket: DatagramSocket? = null
    private var multicastLock: WifiManager.MulticastLock? = null

    fun start(scope: CoroutineScope) {
        if (_isScanning.value) return

        _isScanning.value = true
        acquireMulticastLock()

        listenJob = scope.launch(Dispatchers.IO) {
            try {
                socket = DatagramSocket(DEFAULT_BEACON_PORT).apply {
                    soTimeout = SOCKET_TIMEOUT_MS
                    broadcast = true
                }
                val buffer = ByteArray(RECEIVE_BUFFER_SIZE)

                while (isActive) {
                    try {
                        val packet = DatagramPacket(buffer, buffer.size)
                        socket?.receive(packet)
                        val jsonStr = String(packet.data, 0, packet.length)
                        parseBeacon(jsonStr, packet.address.hostAddress ?: "")?.let { beacon ->
                            val key = "${beacon.hostAddress}:${beacon.restPort}"
                            synchronized(beacons) {
                                beacons[key] = beacon
                                _discoveredBridges.value = beacons.values.toList()
                            }
                        }
                    } catch (_: SocketTimeoutException) {
                        // Normal timeout, continue listening
                    }
                }
            } catch (e: Exception) {
                // Socket creation may fail (port in use, permissions)
            } finally {
                socket?.close()
                socket = null
            }
        }

        cleanupJob = scope.launch(Dispatchers.IO) {
            while (isActive) {
                delay(CLEANUP_INTERVAL_MS)
                val now = System.currentTimeMillis()
                synchronized(beacons) {
                    val stale = beacons.entries.filter { now - it.value.lastSeen > STALE_THRESHOLD_MS }
                    stale.forEach { beacons.remove(it.key) }
                    if (stale.isNotEmpty()) {
                        _discoveredBridges.value = beacons.values.toList()
                    }
                }
            }
        }
    }

    fun stop() {
        _isScanning.value = false
        listenJob?.cancel()
        cleanupJob?.cancel()
        listenJob = null
        cleanupJob = null
        socket?.close()
        socket = null
        releaseMulticastLock()
        synchronized(beacons) {
            beacons.clear()
            _discoveredBridges.value = emptyList()
        }
    }

    private fun parseBeacon(jsonStr: String, sourceAddress: String): BridgeBeacon? {
        return try {
            val obj = json.parseToJsonElement(jsonStr).jsonObject
            val service = obj["service"]?.jsonPrimitive?.content ?: return null
            if (service != "alpine-bridge") return null

            BridgeBeacon(
                service = service,
                protocolVersion = obj["version"]?.jsonPrimitive?.content ?: "0",
                restPort = obj["restPort"]?.jsonPrimitive?.int ?: return null,
                bridgeVersion = obj["bridgeVersion"]?.jsonPrimitive?.content ?: "unknown",
                hostAddress = sourceAddress,
                lastSeen = System.currentTimeMillis()
            )
        } catch (_: Exception) {
            null
        }
    }

    private fun acquireMulticastLock() {
        val wifiManager = context.applicationContext
            .getSystemService(Context.WIFI_SERVICE) as? WifiManager ?: return
        multicastLock = wifiManager.createMulticastLock("alpine_discovery").apply {
            setReferenceCounted(true)
            acquire()
        }
    }

    private fun releaseMulticastLock() {
        multicastLock?.let {
            if (it.isHeld) it.release()
        }
        multicastLock = null
    }
}
