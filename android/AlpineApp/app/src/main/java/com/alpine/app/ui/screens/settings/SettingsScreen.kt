package com.alpine.app.ui.screens.settings

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavController
import com.alpine.app.BuildConfig
import com.alpine.app.data.rpc.TlsMode
import com.alpine.app.data.transport.TransportMode
import com.alpine.app.ui.components.ConnectionIndicator
import com.alpine.app.ui.components.DiscoveredBridgeCard

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    navController: NavController,
    viewModel: SettingsViewModel = hiltViewModel()
) {
    val host by viewModel.host.collectAsState()
    val port by viewModel.port.collectAsState()
    val connectionStatus by viewModel.connectionStatus.collectAsState()
    val statusMessage by viewModel.statusMessage.collectAsState()
    val discoveredBridges by viewModel.discoveredBridges.collectAsState()
    val isScanning by viewModel.isScanning.collectAsState()
    val transportMode by viewModel.transportMode.collectAsState()
    val sharedDirectory by viewModel.sharedDirectory.collectAsState()
    val isBroadcastActive by viewModel.isBroadcastActive.collectAsState()
    val indexedFileCount by viewModel.indexedFileCount.collectAsState()
    val tlsEnabled by viewModel.tlsEnabled.collectAsState()
    val tlsMode by viewModel.tlsMode.collectAsState()
    val tlsCertFingerprint by viewModel.tlsCertFingerprint.collectAsState()
    val apiKey by viewModel.apiKey.collectAsState()

    DisposableEffect(Unit) {
        viewModel.startDiscovery()
        onDispose { viewModel.stopDiscovery() }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Alpine Settings") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                    titleContentColor = MaterialTheme.colorScheme.onPrimaryContainer
                )
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(16.dp)
                .verticalScroll(rememberScrollState())
        ) {
            TransportModeSection(
                selectedMode = transportMode,
                onModeSelected = { viewModel.updateTransportMode(it) }
            )

            Spacer(modifier = Modifier.height(16.dp))

            HorizontalDivider()

            Spacer(modifier = Modifier.height(16.dp))

            when (transportMode) {
                TransportMode.REST_BRIDGE -> {
                    Text(
                        text = "Server Connection",
                        style = MaterialTheme.typography.headlineSmall,
                        color = MaterialTheme.colorScheme.primary
                    )

                    Spacer(modifier = Modifier.height(16.dp))

                    // Nearby Bridges discovery section
                    Text(
                        text = "Nearby Bridges",
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.onSurface
                    )

                    Spacer(modifier = Modifier.height(8.dp))

                    if (isScanning && discoveredBridges.isEmpty()) {
                        Row(verticalAlignment = Alignment.CenterVertically) {
                            CircularProgressIndicator(
                                modifier = Modifier
                                    .height(16.dp)
                                    .width(16.dp),
                                strokeWidth = 2.dp
                            )
                            Spacer(modifier = Modifier.width(8.dp))
                            Text(
                                text = "Scanning for bridges...",
                                style = MaterialTheme.typography.bodyMedium,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }

                    discoveredBridges.forEach { beacon ->
                        DiscoveredBridgeCard(
                            beacon = beacon,
                            onSelect = { viewModel.selectBridge(beacon) }
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                    }

                    if (!isScanning && discoveredBridges.isEmpty()) {
                        Text(
                            text = "No bridges found on local network",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }

                    Spacer(modifier = Modifier.height(16.dp))

                    HorizontalDivider()

                    Spacer(modifier = Modifier.height(16.dp))

                    // Manual Configuration section
                    Text(
                        text = "Manual Configuration",
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.onSurface
                    )

                    Spacer(modifier = Modifier.height(8.dp))

                    OutlinedTextField(
                        value = host,
                        onValueChange = { viewModel.updateHost(it) },
                        label = { Text("Host") },
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth()
                    )

                    Spacer(modifier = Modifier.height(8.dp))

                    OutlinedTextField(
                        value = port,
                        onValueChange = { viewModel.updatePort(it) },
                        label = { Text("Port") },
                        singleLine = true,
                        keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                        modifier = Modifier.fillMaxWidth()
                    )

                    Spacer(modifier = Modifier.height(16.dp))

                    HorizontalDivider()

                    Spacer(modifier = Modifier.height(16.dp))

                    TlsSettingsSection(
                        tlsEnabled = tlsEnabled,
                        onTlsEnabledChanged = { viewModel.updateTlsEnabled(it) },
                        tlsMode = tlsMode,
                        onTlsModeChanged = { viewModel.updateTlsMode(it) },
                        certFingerprint = tlsCertFingerprint,
                        onCertFingerprintChanged = { viewModel.updateTlsCertFingerprint(it) }
                    )

                    Spacer(modifier = Modifier.height(16.dp))
                    HorizontalDivider()
                    Spacer(modifier = Modifier.height(16.dp))

                    Text(
                        text = "Authentication",
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    OutlinedTextField(
                        value = apiKey,
                        onValueChange = { viewModel.updateApiKey(it) },
                        label = { Text("API Key") },
                        placeholder = { Text("Enter server API key") },
                        singleLine = true,
                        modifier = Modifier.fillMaxWidth()
                    )

                    Spacer(modifier = Modifier.height(16.dp))

                    Row(
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        ConnectionIndicator(
                            isConnected = connectionStatus == ConnectionStatus.Connected
                        )
                        Spacer(modifier = Modifier.width(12.dp))
                        OutlinedButton(
                            onClick = { viewModel.testConnection() },
                            enabled = connectionStatus != ConnectionStatus.Testing
                        ) {
                            if (connectionStatus == ConnectionStatus.Testing) {
                                CircularProgressIndicator(
                                    modifier = Modifier
                                        .height(16.dp)
                                        .width(16.dp),
                                    strokeWidth = 2.dp
                                )
                                Spacer(modifier = Modifier.width(8.dp))
                            }
                            Text("Test Connection")
                        }
                    }

                    if (statusMessage.isNotEmpty()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = statusMessage,
                            style = MaterialTheme.typography.bodyMedium,
                            color = when (connectionStatus) {
                                ConnectionStatus.Connected -> MaterialTheme.colorScheme.primary
                                ConnectionStatus.Failed -> MaterialTheme.colorScheme.error
                                else -> MaterialTheme.colorScheme.onSurface
                            }
                        )
                    }
                }

                TransportMode.WIFI_BROADCAST -> {
                    BroadcastSettingsSection(
                        sharedDirectory = sharedDirectory,
                        onDirectoryChanged = { viewModel.updateSharedDirectory(it) },
                        indexedFileCount = indexedFileCount,
                        isBroadcastActive = isBroadcastActive,
                        onStart = { viewModel.startBroadcastServices() },
                        onStop = { viewModel.stopBroadcastServices() },
                        onReindex = { viewModel.reindexContent() }
                    )
                }
            }

            Spacer(modifier = Modifier.weight(1f))

            Button(
                onClick = {
                    if (viewModel.saveAndContinue()) {
                        navController.navigate("dashboard") {
                            popUpTo("settings") { inclusive = true }
                        }
                    }
                },
                modifier = Modifier.fillMaxWidth()
            ) {
                Text("Save & Continue")
            }
        }
    }
}

@Composable
private fun TlsSettingsSection(
    tlsEnabled: Boolean,
    onTlsEnabledChanged: (Boolean) -> Unit,
    tlsMode: TlsMode,
    onTlsModeChanged: (TlsMode) -> Unit,
    certFingerprint: String,
    onCertFingerprintChanged: (String) -> Unit
) {
    Column {
        Text(
            text = "TLS / Security",
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.onSurface
        )

        Spacer(modifier = Modifier.height(8.dp))

        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(text = "Enable TLS", style = MaterialTheme.typography.bodyLarge)
                Text(
                    text = "Use HTTPS for server connections",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }
            Switch(
                checked = tlsEnabled,
                onCheckedChange = onTlsEnabledChanged
            )
        }

        if (tlsEnabled) {
            Spacer(modifier = Modifier.height(12.dp))

            Text(
                text = "Certificate Verification",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurface
            )

            Spacer(modifier = Modifier.height(4.dp))

            val modes = listOf(
                TlsMode.SYSTEM_CA to ("System CA" to "Verify using device trusted certificates"),
                TlsMode.PINNED to ("Certificate Pinning" to "Pin to a specific certificate fingerprint"),
                TlsMode.TRUST_ALL to ("Trust All (Dev)" to "Accept any certificate — insecure")
            ).let { list ->
                if (BuildConfig.DEBUG) list else list.filter { it.first != TlsMode.TRUST_ALL }
            }

            modes.forEach { (mode, labelDesc) ->
                val (label, description) = labelDesc
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .selectable(
                            selected = mode == tlsMode,
                            onClick = { onTlsModeChanged(mode) }
                        )
                        .padding(vertical = 6.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    RadioButton(
                        selected = mode == tlsMode,
                        onClick = null
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Column {
                        Text(text = label, style = MaterialTheme.typography.bodyLarge)
                        Text(
                            text = description,
                            style = MaterialTheme.typography.bodySmall,
                            color = if (mode == TlsMode.TRUST_ALL)
                                MaterialTheme.colorScheme.error
                            else
                                MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                }
            }

            if (tlsMode == TlsMode.PINNED) {
                Spacer(modifier = Modifier.height(8.dp))

                OutlinedTextField(
                    value = certFingerprint,
                    onValueChange = onCertFingerprintChanged,
                    label = { Text("SHA-256 Fingerprint") },
                    placeholder = { Text("sha256/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=") },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}
