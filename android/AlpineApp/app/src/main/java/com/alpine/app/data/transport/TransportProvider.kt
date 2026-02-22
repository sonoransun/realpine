package com.alpine.app.data.transport

import android.app.Application
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.stringPreferencesKey
import com.alpine.app.AlpineApp
import com.alpine.app.data.rpc.AlpineRpcService
import com.alpine.app.data.rpc.TlsConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.data.storage.SecureStorage
import kotlinx.coroutines.flow.first

object TransportProvider {

    private val transportModeKey = stringPreferencesKey("transport_mode")
    private val hostKey = stringPreferencesKey("host")
    private val portKey = stringPreferencesKey("port")
    private val tlsEnabledKey = stringPreferencesKey("tls_enabled")
    private val tlsModeKey = stringPreferencesKey("tls_mode")
    private val tlsCertFingerprintKey = stringPreferencesKey("tls_cert_fingerprint")

    suspend fun createTransport(
        application: Application,
        dataStore: DataStore<Preferences>
    ): QueryTransport {
        val prefs = dataStore.data.first()
        val mode = getTransportMode(prefs)

        return when (mode) {
            TransportMode.REST_BRIDGE -> {
                val host = prefs[hostKey] ?: "10.0.2.2"
                val port = prefs[portKey] ?: "8080"
                val certFp = SecureStorage.read(application, "tls_cert_fingerprint") ?: prefs[tlsCertFingerprintKey] ?: ""
                val tlsConfig = buildTlsConfig(prefs, host, certFp)
                val baseUrl = tlsConfig.buildUrl(host, port)
                val apiKey = SecureStorage.read(application, "api_key") ?: ""
                val rpcService = AlpineRpcService(baseUrl, tlsConfig, host, apiKey)
                JsonRpcTransport(rpcService)
            }
            TransportMode.WIFI_BROADCAST -> (application as AlpineApp).broadcastTransport
        }
    }

    fun getTransportMode(prefs: Preferences): TransportMode {
        return prefs[transportModeKey]
            ?.let {
                try { TransportMode.valueOf(it) } catch (_: Exception) { null }
            }
            ?: TransportMode.REST_BRIDGE
    }

    fun buildTlsConfig(prefs: Preferences, host: String, certFpOverride: String? = null): TlsConfig {
        val tlsEnabled = prefs[tlsEnabledKey] == "true"
        val tlsModeStr = prefs[tlsModeKey] ?: "SYSTEM_CA"
        val certFp = certFpOverride ?: prefs[tlsCertFingerprintKey] ?: ""
        return TlsConfig(
            enabled = tlsEnabled,
            mode = try { TlsMode.valueOf(tlsModeStr) } catch (_: Exception) { TlsMode.SYSTEM_CA },
            certFingerprint = certFp,
            hostname = host
        )
    }
}
