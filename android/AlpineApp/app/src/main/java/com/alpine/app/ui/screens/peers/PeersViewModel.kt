package com.alpine.app.ui.screens.peers

import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.data.model.PeerDetail
import com.alpine.app.data.rpc.AlpineRpcService
import com.alpine.app.data.rpc.TlsConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.data.storage.SecureStorage
import com.alpine.app.data.util.sanitizeError
import com.alpine.app.data.validation.InputValidator
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class PeersViewModel @Inject constructor(
    private val dataStore: DataStore<Preferences>,
    private val secureStorage: SecureStorage
) : ViewModel() {

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
                _message.value = sanitizeError(e)
            }
            _isLoading.value = false
        }
    }

    fun addPeer(ipAddress: String, port: Long) {
        viewModelScope.launch {
            if (!InputValidator.isValidIpAddress(ipAddress)) {
                _message.value = "Invalid IP address"
                return@launch
            }
            if (port < 1 || port > 65535) {
                _message.value = "Invalid port (must be 1-65535)"
                return@launch
            }
            try {
                getService().addPeer(ipAddress, port)
                _message.value = "Peer added"
                loadPeers()
            } catch (e: Exception) {
                _message.value = sanitizeError(e)
            }
        }
    }

    fun pingPeer(peerId: Long) {
        viewModelScope.launch {
            try {
                val ok = getService().pingPeer(peerId)
                _message.value = if (ok) "Ping sent" else "Ping failed"
            } catch (e: Exception) {
                _message.value = sanitizeError(e)
            }
        }
    }

    fun activatePeer(peerId: Long) {
        viewModelScope.launch {
            try {
                getService().activatePeer(peerId)
                _message.value = "Peer activated"
            } catch (e: Exception) {
                _message.value = sanitizeError(e)
            }
        }
    }

    fun deactivatePeer(peerId: Long) {
        viewModelScope.launch {
            try {
                getService().deactivatePeer(peerId)
                _message.value = "Peer deactivated"
            } catch (e: Exception) {
                _message.value = sanitizeError(e)
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
            val certFp = secureStorage.read("tls_cert_fingerprint")
                ?: prefs[stringPreferencesKey("tls_cert_fingerprint")] ?: ""
            val tlsConfig = TlsConfig(
                enabled = tlsEnabled,
                mode = try { TlsMode.valueOf(tlsModeStr) } catch (_: Exception) { TlsMode.SYSTEM_CA },
                certFingerprint = certFp,
                hostname = host
            )
            val apiKey = secureStorage.read("api_key") ?: ""
            rpcService = AlpineRpcService(tlsConfig.buildUrl(host, port), tlsConfig, host, apiKey)
        }
        return rpcService!!
    }

    override fun onCleared() {
        super.onCleared()
        rpcService?.shutdown()
    }
}
