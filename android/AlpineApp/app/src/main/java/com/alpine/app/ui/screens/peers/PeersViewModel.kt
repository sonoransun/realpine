package com.alpine.app.ui.screens.peers

import android.app.Application
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.data.model.PeerDetail
import com.alpine.app.data.rpc.AlpineRpcService
import com.alpine.app.data.rpc.TlsConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.ui.screens.settings.dataStore
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class PeersViewModel(application: Application) : AndroidViewModel(application) {

    private val dataStore = application.dataStore

    private val _peers = MutableStateFlow<List<PeerDetail>>(emptyList())
    val peers: StateFlow<List<PeerDetail>> = _peers.asStateFlow()

    private val _isLoading = MutableStateFlow(true)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _message = MutableStateFlow<String?>(null)
    val message: StateFlow<String?> = _message.asStateFlow()

    private var rpcService: AlpineRpcService? = null

    init {
        loadPeers()
    }

    private fun loadPeers() {
        viewModelScope.launch {
            _isLoading.value = true
            try {
                val svc = getService()
                val peerIds = svc.getAllPeers()
                val details = peerIds.map { svc.getPeer(it) }
                _peers.value = details
            } catch (e: Exception) {
                _message.value = "Failed to load peers: ${e.message}"
            }
            _isLoading.value = false
        }
    }

    fun addPeer(ipAddress: String, port: Long) {
        viewModelScope.launch {
            try {
                getService().addPeer(ipAddress, port)
                _message.value = "Peer added"
                loadPeers()
            } catch (e: Exception) {
                _message.value = "Failed: ${e.message}"
            }
        }
    }

    fun pingPeer(peerId: Long) {
        viewModelScope.launch {
            try {
                val ok = getService().pingPeer(peerId)
                _message.value = if (ok) "Ping sent" else "Ping failed"
            } catch (e: Exception) {
                _message.value = "Ping error: ${e.message}"
            }
        }
    }

    fun activatePeer(peerId: Long) {
        viewModelScope.launch {
            try {
                getService().activatePeer(peerId)
                _message.value = "Peer activated"
            } catch (e: Exception) {
                _message.value = "Failed: ${e.message}"
            }
        }
    }

    fun deactivatePeer(peerId: Long) {
        viewModelScope.launch {
            try {
                getService().deactivatePeer(peerId)
                _message.value = "Peer deactivated"
            } catch (e: Exception) {
                _message.value = "Failed: ${e.message}"
            }
        }
    }

    fun clearMessage() {
        _message.value = null
    }

    private suspend fun getService(): AlpineRpcService {
        if (rpcService == null) {
            val prefs = dataStore.data.first()
            val host = prefs[stringPreferencesKey("host")] ?: "10.0.2.2"
            val port = prefs[stringPreferencesKey("port")] ?: "8080"
            val tlsEnabled = prefs[stringPreferencesKey("tls_enabled")] == "true"
            val tlsModeStr = prefs[stringPreferencesKey("tls_mode")] ?: "SYSTEM_CA"
            val certFp = prefs[stringPreferencesKey("tls_cert_fingerprint")] ?: ""
            val tlsConfig = TlsConfig(
                enabled = tlsEnabled,
                mode = try { TlsMode.valueOf(tlsModeStr) } catch (_: Exception) { TlsMode.SYSTEM_CA },
                certFingerprint = certFp,
                hostname = host
            )
            rpcService = AlpineRpcService(tlsConfig.buildUrl(host, port), tlsConfig, host)
        }
        return rpcService!!
    }

    override fun onCleared() {
        super.onCleared()
        rpcService?.shutdown()
    }
}
