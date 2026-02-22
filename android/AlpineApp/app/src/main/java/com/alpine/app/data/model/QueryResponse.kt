package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class QueryResponse(
    val queryId: Long
)
