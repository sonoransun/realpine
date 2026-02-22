package com.alpine.app.data.rpc

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.jsonObject
import kotlinx.serialization.json.jsonPrimitive
import kotlinx.serialization.json.put
import kotlinx.serialization.json.intOrNull
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.RequestBody.Companion.toRequestBody
import okhttp3.logging.HttpLoggingInterceptor
import com.alpine.app.BuildConfig
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicLong

class JsonRpcClient(
    private val baseUrl: String,
    tlsConfig: TlsConfig = TlsConfig(),
    host: String = "",
    private val apiKey: String = ""
) {
    private val json = Json { ignoreUnknownKeys = true; coerceInputValues = true }
    private val requestId = AtomicLong(1)
    private val jsonMediaType = "application/json; charset=utf-8".toMediaType()

    private val client: OkHttpClient = run {
        val builder = OkHttpClient.Builder()
            .connectTimeout(10, TimeUnit.SECONDS)
            .readTimeout(30, TimeUnit.SECONDS)
            .writeTimeout(15, TimeUnit.SECONDS)
        if (apiKey.isNotBlank()) {
            builder.addInterceptor { chain ->
                val newRequest = chain.request().newBuilder()
                    .addHeader("Authorization", "Bearer $apiKey")
                    .build()
                chain.proceed(newRequest)
            }
        }
        if (BuildConfig.DEBUG) {
            builder.addInterceptor(HttpLoggingInterceptor().apply {
                level = HttpLoggingInterceptor.Level.HEADERS
            })
        }
        tlsConfig.applyTo(builder, host)
        builder.build()
    }

    suspend fun call(method: String, params: JsonObject = buildJsonObject {}): JsonElement {
        return withContext(Dispatchers.IO) {
            val id = requestId.getAndIncrement()
            val request = buildJsonObject {
                put("jsonrpc", "2.0")
                put("method", method)
                put("params", params)
                put("id", id)
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

            val jsonObj = try {
                json.parseToJsonElement(responseBody).jsonObject
            } catch (e: Exception) {
                throw JsonRpcException(-32603, "Invalid JSON response: ${e.message}")
            }

            if ("error" in jsonObj) {
                val error = jsonObj["error"]!!.jsonObject
                throw JsonRpcException(
                    error["code"]?.jsonPrimitive?.intOrNull ?: -1,
                    error["message"]?.jsonPrimitive?.content ?: "Unknown RPC error"
                )
            }

            jsonObj["result"] ?: throw JsonRpcException(-32603, "No result in response")
        }
    }

    fun shutdown() {
        client.dispatcher.executorService.shutdown()
        client.connectionPool.evictAll()
    }
}

class JsonRpcException(val code: Int, message: String) : Exception(message)
