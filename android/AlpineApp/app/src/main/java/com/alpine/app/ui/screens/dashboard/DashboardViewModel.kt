package com.alpine.app.ui.screens.dashboard

import android.app.Application
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.data.model.GroupInfo
import com.alpine.app.data.model.ServerStatus
import com.alpine.app.data.rpc.AlpineRpcService
import com.alpine.app.data.rpc.TlsConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.ui.screens.settings.dataStore
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class DashboardViewModel(application: Application) : AndroidViewModel(application) {

    private val dataStore = application.dataStore

    private val _serverStatus = MutableStateFlow<ServerStatus?>(null)
    val serverStatus: StateFlow<ServerStatus?> = _serverStatus.asStateFlow()

    private val _peerCount = MutableStateFlow(0)
    val peerCount: StateFlow<Int> = _peerCount.asStateFlow()

    private val _groupCount = MutableStateFlow(0)
    val groupCount: StateFlow<Int> = _groupCount.asStateFlow()

    private val _defaultGroup = MutableStateFlow<GroupInfo?>(null)
    val defaultGroup: StateFlow<GroupInfo?> = _defaultGroup.asStateFlow()

    private val _isLoading = MutableStateFlow(true)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()

    private val _isConnected = MutableStateFlow(false)
    val isConnected: StateFlow<Boolean> = _isConnected.asStateFlow()

    private var rpcService: AlpineRpcService? = null

    init {
        refresh()
    }

    fun refresh() {
        viewModelScope.launch {
            _isLoading.value = true
            _error.value = null

            try {
                val prefs = dataStore.data.first()
                val host = prefs[stringPreferencesKey("host")] ?: "10.0.2.2"
                val port = prefs[stringPreferencesKey("port")] ?: "8080"
                val tlsEnabled = prefs[stringPreferencesKey("tls_enabled")] == "true"
                val tlsModeStr = prefs[stringPreferencesKey("tls_mode")] ?: "SYSTEM_CA"
                val certFingerprint = prefs[stringPreferencesKey("tls_cert_fingerprint")] ?: ""

                val tlsConfig = TlsConfig(
                    enabled = tlsEnabled,
                    mode = try { TlsMode.valueOf(tlsModeStr) } catch (_: Exception) { TlsMode.SYSTEM_CA },
                    certFingerprint = certFingerprint,
                    hostname = host
                )

                val baseUrl = tlsConfig.buildUrl(host, port)
                rpcService?.shutdown()
                rpcService = AlpineRpcService(baseUrl, tlsConfig, host)

                val status = rpcService!!.getStatus()
                _serverStatus.value = status
                _isConnected.value = true

                val peers = rpcService!!.getAllPeers()
                _peerCount.value = peers.size

                val groups = rpcService!!.listGroups()
                _groupCount.value = groups.size

                try {
                    _defaultGroup.value = rpcService!!.getDefaultGroupInfo()
                } catch (_: Exception) {}

                _isLoading.value = false
            } catch (e: Exception) {
                _isConnected.value = false
                _error.value = e.message ?: "Connection failed"
                _isLoading.value = false
            }
        }
    }

    override fun onCleared() {
        super.onCleared()
        rpcService?.shutdown()
    }
}
