package com.alpine.app.ui.screens.search

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.data.model.QueryRequest
import com.alpine.app.data.transport.TransportMode
import com.alpine.app.data.transport.TransportProvider
import com.alpine.app.data.util.sanitizeError
import com.alpine.app.data.validation.InputValidator
import dagger.hilt.android.lifecycle.HiltViewModel
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class SearchViewModel @Inject constructor(
    @ApplicationContext private val appContext: Context,
    private val dataStore: DataStore<Preferences>
) : ViewModel() {

    private val _queryString = MutableStateFlow("")
    val queryString: StateFlow<String> = _queryString.asStateFlow()

    private val _groupName = MutableStateFlow("")
    val groupName: StateFlow<String> = _groupName.asStateFlow()

    private val _autoHaltLimit = MutableStateFlow("100")
    val autoHaltLimit: StateFlow<String> = _autoHaltLimit.asStateFlow()

    private val _peerDescMax = MutableStateFlow("50")
    val peerDescMax: StateFlow<String> = _peerDescMax.asStateFlow()

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()

    private val _navigateToResults = MutableSharedFlow<Long>()
    val navigateToResults: SharedFlow<Long> = _navigateToResults.asSharedFlow()

    private val _transportMode = MutableStateFlow(TransportMode.REST_BRIDGE)
    val transportMode: StateFlow<TransportMode> = _transportMode.asStateFlow()

    init {
        viewModelScope.launch {
            val prefs = dataStore.data.first()
            _transportMode.value = TransportProvider.getTransportMode(prefs)
        }
    }

    fun updateQueryString(value: String) {
        _queryString.value = InputValidator.sanitizeQueryString(value)
    }

    fun updateGroupName(value: String) {
        _groupName.value = value
    }

    fun updateAutoHaltLimit(value: String) {
        _autoHaltLimit.value = value
    }

    fun updatePeerDescMax(value: String) {
        _peerDescMax.value = value
    }

    fun clearError() {
        _error.value = null
    }

    fun search() {
        viewModelScope.launch {
            _isLoading.value = true
            _error.value = null

            val transport = TransportProvider.createTransport(
                appContext, dataStore
            )
            val request = QueryRequest(
                queryString = _queryString.value,
                groupName = _groupName.value,
                autoHaltLimit = _autoHaltLimit.value.toLongOrNull() ?: 100,
                peerDescMax = _peerDescMax.value.toLongOrNull() ?: 50
            )

            val result = transport.startQuery(request)
            result.onSuccess { response ->
                _isLoading.value = false
                _navigateToResults.emit(response.queryId)
            }.onFailure { e ->
                _isLoading.value = false
                _error.value = sanitizeError(e)
            }
        }
    }
}
