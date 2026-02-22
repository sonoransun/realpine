import SwiftUI

struct SettingsView: View {
    @State private var viewModel: SettingsViewModel
    @Environment(\.dismiss) private var dismiss

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        _viewModel = State(wrappedValue: SettingsViewModel(
            settings: settings,
            secureStorage: secureStorage,
            discoveryManager: WifiDiscoveryManager(),
            broadcastManager: BroadcastServiceManager()
        ))
    }

    var body: some View {
        Form {
            transportModeSection
            if viewModel.transportMode == .restBridge {
                bridgeDiscoverySection
                connectionSection
                tlsSection
                apiKeySection
                securitySection
                testConnectionSection
            } else {
                wifiBroadcastSection
            }
            saveSection
        }
        .navigationTitle("Settings")
        .navigationBarTitleDisplayMode(.inline)
    }

    // MARK: - Transport Mode

    private var transportModeSection: some View {
        Section("Transport Mode") {
            Picker("Mode", selection: $viewModel.transportMode) {
                ForEach(TransportMode.allCases, id: \.self) { mode in
                    Text(mode == .restBridge ? "REST Bridge" : "WiFi Broadcast")
                        .tag(mode)
                }
            }
            .pickerStyle(.segmented)
        }
    }

    // MARK: - Bridge Discovery

    private var bridgeDiscoverySection: some View {
        Section("Bridge Discovery") {
            if viewModel.isScanning {
                if viewModel.discoveredBridges.isEmpty {
                    HStack {
                        ProgressView()
                            .padding(.trailing, 8)
                        Text("Scanning for bridges...")
                            .font(AlpineTheme.Typography.caption)
                            .foregroundStyle(.secondary)
                    }
                } else {
                    ForEach(viewModel.discoveredBridges) { beacon in
                        DiscoveredBridgeCard(beacon: beacon) {
                            viewModel.selectBridge(beacon)
                        }
                        .listRowInsets(EdgeInsets())
                        .listRowBackground(Color.clear)
                    }
                }
                Button("Stop Scanning") {
                    viewModel.stopDiscovery()
                }
                .foregroundStyle(.red)
            } else {
                Button("Scan for Bridges") {
                    viewModel.startDiscovery()
                }
            }
        }
    }

    // MARK: - Connection

    private var connectionSection: some View {
        Section("Connection") {
            TextField("Host", text: $viewModel.host)
                .textContentType(.URL)
                .autocorrectionDisabled()
                .textInputAutocapitalization(.never)
            TextField("Port", text: $viewModel.port)
                .keyboardType(.numberPad)
        }
    }

    // MARK: - TLS

    private var tlsSection: some View {
        Section("TLS") {
            Toggle("Enable TLS", isOn: $viewModel.tlsEnabled)
            if viewModel.tlsEnabled {
                Picker("TLS Mode", selection: $viewModel.tlsMode) {
                    ForEach(TlsMode.allCases, id: \.self) { mode in
                        Text(tlsModeLabel(mode)).tag(mode)
                    }
                }
                if viewModel.tlsMode == .pinned {
                    TextField("Certificate Fingerprint", text: $viewModel.tlsCertFingerprint)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
                        .font(.system(.body, design: .monospaced))
                }
            }
        }
    }

    private func tlsModeLabel(_ mode: TlsMode) -> String {
        switch mode {
        case .systemCA: "System CA"
        case .pinned: "Certificate Pinning"
        case .trustAll: "Trust All (Insecure)"
        }
    }

    // MARK: - API Key

    private var apiKeySection: some View {
        Section("Authentication") {
            SecureField("API Key", text: $viewModel.apiKey)
                .autocorrectionDisabled()
                .textInputAutocapitalization(.never)
        }
    }

    // MARK: - Security

    private var securitySection: some View {
        Section("Security") {
            Toggle("Require Authentication", isOn: $viewModel.authRequired)

            if viewModel.authRequired {
                Picker("Auth Method", selection: $viewModel.authMethodRaw) {
                    Text("None").tag(AuthMethod.none.rawValue)
                    Text("TOTP").tag(AuthMethod.totp.rawValue)
                    Text("YubiKey").tag(AuthMethod.yubiKey.rawValue)
                    Text("Both").tag(AuthMethod.totpAndYubiKey.rawValue)
                }

                Picker("Auto-Lock", selection: $viewModel.autoLockTimeout) {
                    Text("Immediately").tag(0)
                    Text("1 minute").tag(60)
                    Text("5 minutes").tag(300)
                    Text("30 minutes").tag(1800)
                    Text("Never").tag(-1)
                }

                NavigationLink("Set Up Authentication") {
                    AuthEnrollmentView(
                        secureStorage: viewModel.exposedSecureStorage,
                        settingsStore: viewModel.exposedSettingsStore
                    )
                }

                Button("Remove Authentication", role: .destructive) {
                    viewModel.removeAuthentication()
                }
            }
        }
    }

    // MARK: - Test Connection

    private var testConnectionSection: some View {
        Section("Connection Test") {
            Button {
                viewModel.testConnection()
            } label: {
                HStack {
                    Text("Test Connection")
                    Spacer()
                    switch viewModel.connectionStatus {
                    case .unknown:
                        EmptyView()
                    case .testing:
                        ProgressView()
                    case .connected:
                        Image(systemName: "checkmark.circle.fill")
                            .foregroundStyle(.green)
                    case .failed:
                        Image(systemName: "xmark.circle.fill")
                            .foregroundStyle(.red)
                    }
                }
            }
            if !viewModel.statusMessage.isEmpty {
                Text(viewModel.statusMessage)
                    .font(AlpineTheme.Typography.caption)
                    .foregroundStyle(
                        viewModel.connectionStatus == .connected ? .green : .secondary
                    )
            }
        }
    }

    // MARK: - WiFi Broadcast

    private var wifiBroadcastSection: some View {
        Section("WiFi Broadcast") {
            TextField("Shared Directory", text: $viewModel.sharedDirectory)
                .autocorrectionDisabled()
                .textInputAutocapitalization(.never)

            HStack {
                Text("Indexed Files")
                Spacer()
                Text("\(viewModel.indexedFileCount)")
                    .foregroundStyle(.secondary)
            }

            if viewModel.isBroadcastActive {
                Button("Stop Broadcast") {
                    viewModel.stopBroadcastServices()
                }
                .foregroundStyle(.red)

                Button("Re-index Content") {
                    viewModel.reindexContent()
                }
            } else {
                Button("Start Broadcast") {
                    viewModel.startBroadcastServices()
                }
            }
        }
    }

    // MARK: - Save

    private var saveSection: some View {
        Section {
            Button {
                viewModel.save()
                dismiss()
            } label: {
                Text("Save Settings")
                    .frame(maxWidth: .infinity)
                    .font(AlpineTheme.Typography.headline)
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
            .listRowBackground(Color.clear)
            .listRowInsets(EdgeInsets())
        }
    }
}
