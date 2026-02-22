package com.alpine.app.data.model

import kotlinx.serialization.Serializable

object FilterModels {
    @Serializable
    data class Subnet(
        val ipAddress: String,
        val netMask: String
    )
}
