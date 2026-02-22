package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class QueryRequest(
    val queryString: String,
    val groupName: String = "",
    val autoHaltLimit: Long = 100,
    val peerDescMax: Long = 50
)
