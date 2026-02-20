package com.alpine.app.ui.screens.settings

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.selection.selectable
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.alpine.app.data.transport.TransportMode

@Composable
fun TransportModeSection(
    selectedMode: TransportMode,
    onModeSelected: (TransportMode) -> Unit
) {
    Column {
        Text(
            text = "Transport Mode",
            style = MaterialTheme.typography.headlineSmall,
            color = MaterialTheme.colorScheme.primary
        )

        Spacer(modifier = Modifier.height(8.dp))

        TransportMode.entries.forEach { mode ->
            val (label, description) = when (mode) {
                TransportMode.REST_BRIDGE -> "REST Bridge" to "Connect through Alpine REST bridge server"
                TransportMode.WIFI_BROADCAST -> "WiFi Broadcast" to "Discover peers directly on local WiFi"
            }
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .selectable(
                        selected = mode == selectedMode,
                        onClick = { onModeSelected(mode) }
                    )
                    .padding(vertical = 8.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                RadioButton(
                    selected = mode == selectedMode,
                    onClick = null
                )
                Spacer(modifier = Modifier.width(8.dp))
                Column {
                    Text(text = label, style = MaterialTheme.typography.bodyLarge)
                    Text(
                        text = description,
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}
