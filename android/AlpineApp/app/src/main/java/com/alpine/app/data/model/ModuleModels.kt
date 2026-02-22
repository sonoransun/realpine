package com.alpine.app.data.model

import kotlinx.serialization.Serializable

@Serializable
data class ModuleInfo(
    val moduleId: Long,
    val moduleName: String,
    val description: String,
    val version: String,
    val libraryPath: String,
    val bootstrapSymbol: String,
    val activeTime: Long
)
