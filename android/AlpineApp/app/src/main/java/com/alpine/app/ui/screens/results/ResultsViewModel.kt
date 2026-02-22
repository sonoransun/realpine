package com.alpine.app.ui.screens.results

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.lifecycle.SavedStateHandle
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.alpine.app.data.model.PeerResources
import com.alpine.app.data.model.QueryStatusResponse
import com.alpine.app.data.transport.QueryTransport
import com.alpine.app.data.transport.TransportProvider
import com.alpine.app.data.util.sanitizeError
import dagger.hilt.android.lifecycle.HiltViewModel
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class ResultsViewModel @Inject constructor(
    @ApplicationContext private val appContext: Context,
    private val dataStore: DataStore<Preferences>,
    savedStateHandle: SavedStateHandle
) : ViewModel() {

    private val queryId: Long = savedStateHandle["queryId"] ?: 0L

    private val _queryStatus = MutableStateFlow<QueryStatusResponse?>(null)
    val queryStatus: StateFlow<QueryStatusResponse?> = _queryStatus.asStateFlow()

    private val _results = MutableStateFlow<List<PeerResources>>(emptyList())
    val results: StateFlow<List<PeerResources>> = _results.asStateFlow()

    private val _isLoading = MutableStateFlow(true)
    val isLoading: StateFlow<Boolean> = _isLoading.asStateFlow()

    private val _error = MutableStateFlow<String?>(null)
    val error: StateFlow<String?> = _error.asStateFlow()

    private var pollingJob: Job? = null
    private var transport: QueryTransport? = null

    init {
        viewModelScope.launch {
            transport = TransportProvider.createTransport(appContext, dataStore)
            startPolling()
        }
    }

    private fun startPolling() {
        pollingJob = viewModelScope.launch {
            val t = transport ?: return@launch
            while (true) {
                val statusResult = t.getQueryStatus(queryId)
                statusResult.onSuccess { status ->
                    _queryStatus.value = status
                    _isLoading.value = false
                }.onFailure { e ->
                    _error.value = sanitizeError(e)
                    _isLoading.value = false
                    return@launch
                }

                val resultsResult = t.getQueryResults(queryId)
                resultsResult.onSuccess { response ->
                    _results.value = response.peers
                }

                val currentStatus = _queryStatus.value
                if (currentStatus != null && !currentStatus.inProgress) {
                    break
                }

                delay(2000)
            }
        }
    }

    fun cancelQuery() {
        viewModelScope.launch {
            transport?.cancelQuery(queryId)
            stopPolling()
        }
    }

    fun stopPolling() {
        pollingJob?.cancel()
        pollingJob = null
    }

    override fun onCleared() {
        super.onCleared()
        stopPolling()
    }
}
