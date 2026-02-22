package com.alpine.app.data.model

object FilterModels {
    data class Subnet(
        val ipAddress: String,
        val netMask: String
    )
}
