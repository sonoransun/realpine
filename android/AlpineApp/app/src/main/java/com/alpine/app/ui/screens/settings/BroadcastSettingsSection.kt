package com.alpine.app.ui.screens.settings

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun BroadcastSettingsSection(
    sharedDirectory: String,
    onDirectoryChanged: (String) -> Unit,
    indexedFileCount: Int,
    isBroadcastActive: Boolean,
    onStart: () -> Unit,
    onStop: () -> Unit,
    onReindex: () -> Unit
) {
    Column {
        Text(
            text = "Broadcast Settings",
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.onSurface
        )

        Spacer(modifier = Modifier.height(8.dp))

        OutlinedTextField(
            value = sharedDirectory,
            onValueChange = onDirectoryChanged,
            label = { Text("Shared Directory") },
            singleLine = true,
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(modifier = Modifier.height(8.dp))

        Text(
            text = "Indexed files: $indexedFileCount",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )

        Spacer(modifier = Modifier.height(12.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            if (isBroadcastActive) {
                OutlinedButton(onClick = onStop) {
                    Text("Stop Services")
                }
            } else {
                Button(onClick = onStart) {
                    Text("Start Services")
                }
            }
            OutlinedButton(onClick = onReindex) {
                Text("Reindex")
            }
        }

        if (isBroadcastActive) {
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                text = "Broadcast services running",
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.primary
            )
        }
    }
}
