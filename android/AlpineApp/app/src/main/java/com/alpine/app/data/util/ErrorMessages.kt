package com.alpine.app.data.util

import com.alpine.app.data.rpc.JsonRpcException

fun sanitizeError(e: Throwable): String = when (e) {
    is java.net.ConnectException -> "Unable to connect to server"
    is java.net.SocketTimeoutException -> "Connection timed out"
    is JsonRpcException -> if (e.code == 401) "Authentication failed" else "Server error (code ${e.code})"
    is javax.net.ssl.SSLException -> "TLS connection failed"
    else -> "An unexpected error occurred"
}
