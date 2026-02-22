package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class ServerStatus(
    val status: String,
    val version: String
)
