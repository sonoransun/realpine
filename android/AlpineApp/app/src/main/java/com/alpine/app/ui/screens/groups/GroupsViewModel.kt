package com.alpine.app.ui.screens.groups

import android.app.Application
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.data.model.GroupInfo
import com.alpine.app.data.rpc.AlpineRpcService
import com.alpine.app.data.rpc.TlsConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.data.storage.SecureStorage
import com.alpine.app.data.util.sanitizeError
import com.alpine.app.ui.screens.settings.dataStore
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

class GroupsViewModel(application: Application) : AndroidViewModel(application) {

    private val dataStore = application.dataStore

    private val _groups = MutableStateFlow<List<GroupInfo>>(emptyList())
    val groups: StateFlow<List<GroupInfo>> = _groups.asStateFlow()

    private val _isLoading = MutableStateFlow(true)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _message = MutableStateFlow<String?>(null)
    val message: StateFlow<String?> = _message.asStateFlow()

    private var rpcService: AlpineRpcService? = null

    init {
        loadGroups()
    }

    private fun loadGroups() {
        viewModelScope.launch {
            _isLoading.value = true
            try {
                val svc = getService()
                val groupIds = svc.listGroups()
                val infos = groupIds.map { svc.getGroupInfo(it) }
                _groups.value = infos
            } catch (e: Exception) {
                _message.value = sanitizeError(e)
            }
            _isLoading.value = false
        }
    }

    fun createGroup(name: String, description: String) {
        viewModelScope.launch {
            try {
                getService().createGroup(name, description)
                _message.value = "Group created"
                loadGroups()
            } catch (e: Exception) {
                _message.value = sanitizeError(e)
            }
        }
    }

    fun deleteGroup(groupId: Long) {
        viewModelScope.launch {
            try {
                getService().deleteGroup(groupId)
                _message.value = "Group deleted"
                loadGroups()
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
            val certFp = SecureStorage.read(getApplication(), "tls_cert_fingerprint")
                ?: prefs[stringPreferencesKey("tls_cert_fingerprint")] ?: ""
            val tlsConfig = TlsConfig(
                enabled = tlsEnabled,
                mode = try { TlsMode.valueOf(tlsModeStr) } catch (_: Exception) { TlsMode.SYSTEM_CA },
                certFingerprint = certFp,
                hostname = host
            )
            val apiKey = SecureStorage.read(getApplication(), "api_key") ?: ""
            rpcService = AlpineRpcService(tlsConfig.buildUrl(host, port), tlsConfig, host, apiKey)
        }
        return rpcService!!
    }

    override fun onCleared() {
        super.onCleared()
        rpcService?.shutdown()
    }
}
