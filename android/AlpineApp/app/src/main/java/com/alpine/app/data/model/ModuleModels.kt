package com.alpine.app.data.model

data class ModuleInfo(
    val moduleId: Long,
    val moduleName: String,
    val description: String,
    val version: String,
    val libraryPath: String,
    val bootstrapSymbol: String,
    val activeTime: Long
)
