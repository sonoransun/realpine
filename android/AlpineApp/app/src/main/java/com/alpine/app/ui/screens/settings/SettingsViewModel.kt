package com.alpine.app.ui.screens.settings

import android.app.Application
import android.os.Environment
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.AlpineApp
import com.alpine.app.data.discovery.BridgeBeacon
import com.alpine.app.data.discovery.WifiDiscoveryManager
import com.alpine.app.data.rpc.AlpineRpcService
import com.alpine.app.data.rpc.TlsConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.data.transport.TransportMode
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

val Application.dataStore: DataStore<Preferences> by preferencesDataStore(name = "alpine_settings")

enum class ConnectionStatus {
    Idle, Testing, Connected, Failed
}

class SettingsViewModel(application: Application) : AndroidViewModel(application) {

    private val dataStore = application.dataStore
    private val hostKey = stringPreferencesKey("host")
    private val portKey = stringPreferencesKey("port")
    private val transportModeKey = stringPreferencesKey("transport_mode")
    private val sharedDirectoryKey = stringPreferencesKey("shared_directory")
    private val tlsEnabledKey = stringPreferencesKey("tls_enabled")
    private val tlsModeKey = stringPreferencesKey("tls_mode")
    private val tlsCertFingerprintKey = stringPreferencesKey("tls_cert_fingerprint")

    private val _host = MutableStateFlow("10.0.2.2")
    val host: StateFlow<String> = _host.asStateFlow()

    private val _port = MutableStateFlow("8080")
    val port: StateFlow<String> = _port.asStateFlow()

    private val _connectionStatus = MutableStateFlow(ConnectionStatus.Idle)
    val connectionStatus: StateFlow<ConnectionStatus> = _connectionStatus.asStateFlow()

    private val _statusMessage = MutableStateFlow("")
    val statusMessage: StateFlow<String> = _statusMessage.asStateFlow()

    private val discoveryManager = WifiDiscoveryManager(application)

    val discoveredBridges: StateFlow<List<BridgeBeacon>> = discoveryManager.discoveredBridges
    val isScanning: StateFlow<Boolean> = discoveryManager.isScanning

    private val _transportMode = MutableStateFlow(TransportMode.REST_BRIDGE)
    val transportMode: StateFlow<TransportMode> = _transportMode.asStateFlow()

    private val _sharedDirectory = MutableStateFlow(
        Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).absolutePath
    )
    val sharedDirectory: StateFlow<String> = _sharedDirectory.asStateFlow()

    private val broadcastServiceManager = (application as AlpineApp).broadcastServiceManager

    val isBroadcastActive: StateFlow<Boolean> = broadcastServiceManager.isActive
    val indexedFileCount: StateFlow<Int> = broadcastServiceManager.indexedFileCount

    // TLS settings
    private val _tlsEnabled = MutableStateFlow(false)
    val tlsEnabled: StateFlow<Boolean> = _tlsEnabled.asStateFlow()

    private val _tlsMode = MutableStateFlow(TlsMode.SYSTEM_CA)
    val tlsMode: StateFlow<TlsMode> = _tlsMode.asStateFlow()

    private val _tlsCertFingerprint = MutableStateFlow("")
    val tlsCertFingerprint: StateFlow<String> = _tlsCertFingerprint.asStateFlow()

    init {
        viewModelScope.launch {
            val prefs = dataStore.data.first()
            prefs[hostKey]?.let { _host.value = it }
            prefs[portKey]?.let { _port.value = it }
            prefs[transportModeKey]?.let { modeName ->
                try {
                    _transportMode.value = TransportMode.valueOf(modeName)
                } catch (_: Exception) {}
            }
            prefs[sharedDirectoryKey]?.let { _sharedDirectory.value = it }
            _tlsEnabled.value = prefs[tlsEnabledKey] == "true"
            prefs[tlsModeKey]?.let { modeName ->
                try {
                    _tlsMode.value = TlsMode.valueOf(modeName)
                } catch (_: Exception) {}
            }
            prefs[tlsCertFingerprintKey]?.let { _tlsCertFingerprint.value = it }
        }
    }

    fun updateHost(host: String) {
        _host.value = host
    }

    fun updatePort(port: String) {
        _port.value = port
    }

    fun updateTransportMode(mode: TransportMode) {
        _transportMode.value = mode
    }

    fun updateSharedDirectory(path: String) {
        _sharedDirectory.value = path
    }

    fun updateTlsEnabled(enabled: Boolean) {
        _tlsEnabled.value = enabled
    }

    fun updateTlsMode(mode: TlsMode) {
        _tlsMode.value = mode
    }

    fun updateTlsCertFingerprint(fingerprint: String) {
        _tlsCertFingerprint.value = fingerprint
    }

    fun startDiscovery() {
        discoveryManager.start(viewModelScope)
    }

    fun stopDiscovery() {
        discoveryManager.stop()
    }

    fun selectBridge(beacon: BridgeBeacon) {
        _host.value = beacon.hostAddress
        _port.value = beacon.restPort.toString()
    }

    fun testConnection() {
        viewModelScope.launch {
            _connectionStatus.value = ConnectionStatus.Testing
            _statusMessage.value = ""

            try {
                val tlsConfig = TlsConfig(
                    enabled = _tlsEnabled.value,
                    mode = _tlsMode.value,
                    certFingerprint = _tlsCertFingerprint.value,
                    hostname = _host.value
                )
                val baseUrl = tlsConfig.buildUrl(_host.value, _port.value)
                val rpc = AlpineRpcService(baseUrl, tlsConfig, _host.value)
                val status = rpc.getStatus()
                _connectionStatus.value = ConnectionStatus.Connected
                _statusMessage.value = "Connected via JSON-RPC - ${status.version}"
                rpc.shutdown()
            } catch (e: Exception) {
                _connectionStatus.value = ConnectionStatus.Failed
                _statusMessage.value = e.message ?: "Connection failed"
            }
        }
    }

    fun startBroadcastServices() {
        broadcastServiceManager.updateSharedDirectory(_sharedDirectory.value)
        broadcastServiceManager.start()
    }

    fun stopBroadcastServices() {
        broadcastServiceManager.stop()
    }

    fun reindexContent() {
        broadcastServiceManager.updateSharedDirectory(_sharedDirectory.value)
    }

    fun saveAndContinue() {
        viewModelScope.launch {
            dataStore.edit { prefs ->
                prefs[hostKey] = _host.value
                prefs[portKey] = _port.value
                prefs[transportModeKey] = _transportMode.value.name
                prefs[sharedDirectoryKey] = _sharedDirectory.value
                prefs[tlsEnabledKey] = if (_tlsEnabled.value) "true" else "false"
                prefs[tlsModeKey] = _tlsMode.value.name
                prefs[tlsCertFingerprintKey] = _tlsCertFingerprint.value
            }
            if (_transportMode.value == TransportMode.WIFI_BROADCAST) {
                broadcastServiceManager.updateSharedDirectory(_sharedDirectory.value)
            }
        }
    }

    override fun onCleared() {
        super.onCleared()
        discoveryManager.stop()
    }
}
