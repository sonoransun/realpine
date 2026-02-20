package com.alpine.app.data.broadcast

import android.os.Environment
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import java.io.File

class BroadcastServiceManager {

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.IO)
    private val defaultDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS)
    private val contentProvider = LocalContentProvider(defaultDir)
    private val fileServer = LocalFileServer(defaultDir)
    private val responder = BroadcastResponder(contentProvider)

    private val _isActive = MutableStateFlow(false)
    val isActive: StateFlow<Boolean> = _isActive.asStateFlow()

    private val _indexedFileCount = MutableStateFlow(0)
    val indexedFileCount: StateFlow<Int> = _indexedFileCount.asStateFlow()

    fun start() {
        if (_isActive.value) return
        contentProvider.reindex()
        _indexedFileCount.value = contentProvider.fileCount
        fileServer.start(scope)
        responder.start(scope)
        _isActive.value = true
    }

    fun stop() {
        responder.stop()
        fileServer.stop()
        _isActive.value = false
    }

    fun updateSharedDirectory(path: String) {
        val dir = File(path)
        contentProvider.updateDirectory(dir)
        fileServer.sharedDirectory = dir
        _indexedFileCount.value = contentProvider.fileCount
    }

    fun reindex() {
        contentProvider.reindex()
        _indexedFileCount.value = contentProvider.fileCount
    }
}
