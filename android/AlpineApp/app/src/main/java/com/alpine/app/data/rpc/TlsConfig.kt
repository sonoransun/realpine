package com.alpine.app.data.rpc

import okhttp3.CertificatePinner
import okhttp3.OkHttpClient
import java.security.SecureRandom
import java.security.cert.X509Certificate
import javax.net.ssl.SSLContext
import javax.net.ssl.TrustManager
import javax.net.ssl.X509TrustManager

enum class TlsMode {
    SYSTEM_CA,
    PINNED,
    TRUST_ALL
}

data class TlsConfig(
    val enabled: Boolean = false,
    val mode: TlsMode = TlsMode.SYSTEM_CA,
    val certFingerprint: String = "",
    val hostname: String = ""
) {
    fun applyTo(builder: OkHttpClient.Builder, host: String): OkHttpClient.Builder {
        if (!enabled) return builder

        return when (mode) {
            TlsMode.SYSTEM_CA -> builder

            TlsMode.PINNED -> {
                if (certFingerprint.isBlank()) return builder
                val pin = if (certFingerprint.startsWith("sha256/")) {
                    certFingerprint
                } else {
                    "sha256/$certFingerprint"
                }
                val pinner = CertificatePinner.Builder()
                    .add(hostname.ifBlank { host }, pin)
                    .build()
                builder.certificatePinner(pinner)
            }

            TlsMode.TRUST_ALL -> {
                val trustAllManager = object : X509TrustManager {
                    override fun checkClientTrusted(chain: Array<X509Certificate>, authType: String) {}
                    override fun checkServerTrusted(chain: Array<X509Certificate>, authType: String) {}
                    override fun getAcceptedIssuers(): Array<X509Certificate> = arrayOf()
                }
                val sslContext = SSLContext.getInstance("TLS").apply {
                    init(null, arrayOf<TrustManager>(trustAllManager), SecureRandom())
                }
                builder
                    .sslSocketFactory(sslContext.socketFactory, trustAllManager)
                    .hostnameVerifier { _, _ -> true }
            }
        }
    }

    fun buildUrl(host: String, port: String): String {
        val scheme = if (enabled) "https" else "http"
        return "$scheme://$host:$port"
    }
}
