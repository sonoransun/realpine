package com.alpine.app.data.broadcast

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.io.File
import java.net.ServerSocket
import java.net.Socket
import java.net.URLDecoder

class LocalFileServer(
    var sharedDirectory: File,
    private val port: Int = 8091
) {
    private var serverSocket: ServerSocket? = null
    private var serverJob: Job? = null

    fun start(scope: CoroutineScope) {
        serverJob = scope.launch(Dispatchers.IO) {
            serverSocket = ServerSocket(port).apply { soTimeout = 1000 }
            while (isActive) {
                try {
                    val client = serverSocket?.accept() ?: break
                    launch { handleClient(client) }
                } catch (_: java.net.SocketTimeoutException) {
                } catch (_: Exception) {
                    if (!isActive) break
                }
            }
        }
    }

    fun stop() {
        serverJob?.cancel()
        serverJob = null
        try { serverSocket?.close() } catch (_: Exception) {}
        serverSocket = null
    }

    private fun handleClient(client: Socket) {
        client.use { socket ->
            try {
                val requestLine = socket.getInputStream().bufferedReader().readLine() ?: return
                val parts = requestLine.split(" ")
                if (parts.size < 2 || parts[0] != "GET") {
                    sendResponse(socket, 405, "Method Not Allowed", "Only GET supported")
                    return
                }

                val path = URLDecoder.decode(parts[1], "UTF-8")
                if (!path.startsWith("/files/")) {
                    sendResponse(socket, 404, "Not Found", "Not found")
                    return
                }

                // Reject path traversal
                if (path.split("/").any { it == ".." }) {
                    sendResponse(socket, 403, "Forbidden", "Access denied")
                    return
                }

                val file = File(sharedDirectory, path.removePrefix("/files/"))
                if (!file.canonicalPath.startsWith(sharedDirectory.canonicalPath + File.separator)
                    && file.canonicalPath != sharedDirectory.canonicalPath) {
                    sendResponse(socket, 403, "Forbidden", "Access denied")
                    return
                }
                if (!file.exists() || !file.isFile) {
                    sendResponse(socket, 404, "Not Found", "File not found")
                    return
                }

                val output = socket.getOutputStream()
                val contentType = guessContentType(file.name)
                output.write("HTTP/1.1 200 OK\r\nContent-Type: $contentType\r\nContent-Length: ${file.length()}\r\nConnection: close\r\n\r\n".toByteArray())
                file.inputStream().use { it.copyTo(output) }
                output.flush()
            } catch (_: Exception) {}
        }
    }

    private fun sendResponse(socket: Socket, code: Int, status: String, body: String) {
        try {
            val output = socket.getOutputStream()
            output.write("HTTP/1.1 $code $status\r\nContent-Type: text/plain\r\nContent-Length: ${body.length}\r\nConnection: close\r\n\r\n".toByteArray())
            output.write(body.toByteArray())
            output.flush()
        } catch (_: Exception) {}
    }

    private fun guessContentType(fileName: String): String {
        return when (fileName.substringAfterLast('.', "").lowercase()) {
            "html", "htm" -> "text/html"
            "txt" -> "text/plain"
            "json" -> "application/json"
            "jpg", "jpeg" -> "image/jpeg"
            "png" -> "image/png"
            "gif" -> "image/gif"
            "mp4" -> "video/mp4"
            "mp3" -> "audio/mpeg"
            "pdf" -> "application/pdf"
            "zip" -> "application/zip"
            else -> "application/octet-stream"
        }
    }
}
