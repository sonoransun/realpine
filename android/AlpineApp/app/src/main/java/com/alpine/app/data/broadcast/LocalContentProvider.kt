package com.alpine.app.data.broadcast

import com.alpine.app.data.model.ResourceDesc
import java.io.File

class LocalContentProvider(private var sharedDirectory: File) {

    @Volatile private var indexedFiles: List<File> = emptyList()

    val fileCount: Int get() = indexedFiles.size

    fun reindex() {
        indexedFiles = if (sharedDirectory.exists() && sharedDirectory.isDirectory) {
            sharedDirectory.walk().filter { it.isFile }.toList()
        } else {
            emptyList()
        }
    }

    fun updateDirectory(directory: File) {
        sharedDirectory = directory
        reindex()
    }

    fun search(queryString: String, localAddress: String, fileServerPort: Int): List<ResourceDesc> {
        if (queryString.isBlank()) return emptyList()

        val pattern = try {
            Regex(
                queryString.replace(".", "\\.").replace("*", ".*").replace("?", "."),
                RegexOption.IGNORE_CASE
            )
        } catch (_: Exception) { null }

        return indexedFiles
            .filter { file ->
                pattern?.containsMatchIn(file.name)
                    ?: file.name.contains(queryString, ignoreCase = true)
            }
            .mapIndexed { index, file ->
                ResourceDesc(
                    resourceId = (index + 1).toLong(),
                    size = file.length(),
                    description = file.name,
                    locators = listOf("http://$localAddress:$fileServerPort/files/${file.relativeTo(sharedDirectory).path}")
                )
            }
    }
}
