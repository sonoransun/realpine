package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class GroupInfo(
    val groupId: Long,
    val groupName: String,
    val description: String,
    val numPeers: Long,
    val totalQueries: Long,
    val totalResponses: Long
)
