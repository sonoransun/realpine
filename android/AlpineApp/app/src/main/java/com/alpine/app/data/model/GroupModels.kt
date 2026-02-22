package com.alpine.app.data.model

data class GroupInfo(
    val groupId: Long,
    val groupName: String,
    val description: String,
    val numPeers: Long,
    val totalQueries: Long,
    val totalResponses: Long
)
