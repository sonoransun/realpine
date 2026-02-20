package com.alpine.app.ui.components

import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.size
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Cloud
import androidx.compose.material.icons.filled.Wifi
import androidx.compose.material3.AssistChip
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.alpine.app.data.transport.TransportMode

@Composable
fun TransportModeIndicator(mode: TransportMode) {
    val (icon, label) = when (mode) {
        TransportMode.REST_BRIDGE -> Icons.Default.Cloud to "REST"
        TransportMode.WIFI_BROADCAST -> Icons.Default.Wifi to "WiFi"
    }

    AssistChip(
        onClick = {},
        label = { Text(label, style = MaterialTheme.typography.labelSmall) },
        leadingIcon = {
            Icon(imageVector = icon, contentDescription = null, modifier = Modifier.size(16.dp))
        },
        modifier = Modifier.height(28.dp)
    )
}
