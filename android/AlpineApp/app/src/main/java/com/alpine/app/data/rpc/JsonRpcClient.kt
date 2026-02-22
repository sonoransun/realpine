package com.alpine.app.data.rpc

import com.google.gson.Gson
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.google.gson.JsonParser
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import okhttp3.logging.HttpLoggingInterceptor
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicLong

class JsonRpcClient(
    private val baseUrl: String,
    tlsConfig: TlsConfig = TlsConfig(),
    host: String = ""
) {
    private val gson = Gson()
    private val requestId = AtomicLong(1)
    private val jsonMediaType = "application/json; charset=utf-8".toMediaType()

    private val client: OkHttpClient = run {
        val builder = OkHttpClient.Builder()
            .connectTimeout(10, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .writeTimeout(15, TimeUnit.SECONDS)
            .addInterceptor(HttpLoggingInterceptor().apply {
                level = HttpLoggingInterceptor.Level.BODY
            })
        tlsConfig.applyTo(builder, host)
        builder.build()
    }

    suspend fun call(method: String, params: JsonObject = JsonObject()): JsonElement {
        return withContext(Dispatchers.IO) {
            val id = requestId.getAndIncrement()
            val request = JsonObject().apply {
                addProperty("jsonrpc", "2.0")
                addProperty("method", method)
                add("params", params)
                addProperty("id", id)
            }

            val body = request.toString().toRequestBody(jsonMediaType)
            val httpRequest = Request.Builder()
                .url("$baseUrl/rpc")
                .post(body)
                .build()

            val response = client.newCall(httpRequest).execute()
            val responseBody = response.body?.string()
                ?: throw JsonRpcException(-32600, "Empty response from server")

            if (!response.isSuccessful) {
                throw JsonRpcException(
                    response.code,
                    "HTTP ${response.code}: ${response.message}"
                )
            }

            val json = JsonParser.parseString(responseBody).asJsonObject

            if (json.has("error")) {
                val error = json.getAsJsonObject("error")
                throw JsonRpcException(
                    error.get("code")?.asInt ?: -1,
                    error.get("message")?.asString ?: "Unknown RPC error"
                )
            }

            json.get("result") ?: throw JsonRpcException(-32603, "No result in response")
        }
    }

    fun shutdown() {
        client.dispatcher.executorService.shutdown()
        client.connectionPool.evictAll()
    }
}

class JsonRpcException(val code: Int, message: String) : Exception(message)
