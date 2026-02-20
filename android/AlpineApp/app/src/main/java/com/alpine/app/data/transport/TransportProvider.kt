package com.alpine.app.data.transport

import android.app.Application
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.stringPreferencesKey
import com.alpine.app.AlpineApp
import kotlinx.coroutines.flow.first

object TransportProvider {

    private val transportModeKey = stringPreferencesKey("transport_mode")
    private val hostKey = stringPreferencesKey("host")
    private val portKey = stringPreferencesKey("port")

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
                RestBridgeTransport("http://$host:$port/")
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
}
