package com.alpine.app

import android.app.Application
import androidx.datastore.preferences.core.stringPreferencesKey
import com.alpine.app.data.broadcast.BroadcastServiceManager
import com.alpine.app.data.transport.BroadcastTransport
import com.alpine.app.data.transport.TransportMode
import com.alpine.app.data.transport.TransportProvider
import com.alpine.app.ui.screens.settings.dataStore
import dagger.hilt.android.HiltAndroidApp
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

@HiltAndroidApp
class AlpineApp : Application() {

    val broadcastServiceManager by lazy { BroadcastServiceManager() }
    val broadcastTransport by lazy { BroadcastTransport() }

    private val appScope = CoroutineScope(SupervisorJob() + Dispatchers.IO)

    override fun onCreate() {
        super.onCreate()
        appScope.launch {
            val prefs = dataStore.data.first()
            if (TransportProvider.getTransportMode(prefs) == TransportMode.WIFI_BROADCAST) {
                prefs[stringPreferencesKey("shared_directory")]?.let {
                    broadcastServiceManager.updateSharedDirectory(it)
                }
                broadcastServiceManager.start()
            }
        }
    }
}
