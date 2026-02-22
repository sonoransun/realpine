import SwiftUI

/// Wraps app content with an authentication gate.
/// Shows lock screen when auth is required and user is not authenticated.
struct AuthGateView<Content: View>: View {
    @Environment(AuthManager.self) private var authManager
    @Environment(\.scenePhase) private var scenePhase
    let content: () -> Content

    var body: some View {
        Group {
            if authManager.authMethod == .none || !authManager.isLocked {
                content()
            } else {
                AuthLockScreen()
                    .environment(authManager)
            }
        }
        .onChange(of: scenePhase) { _, newPhase in
            authManager.checkAutoLock(phase: newPhase)
        }
    }
}

/// The lock screen shown when authentication is required.
private struct AuthLockScreen: View {
    @Environment(AuthManager.self) private var authManager
    @State private var viewModel: AuthViewModel?
    @State private var biometricError: String?

    var body: some View {
        NavigationStack {
            VStack(spacing: 32) {
                Spacer()

                Image(systemName: "lock.shield.fill")
                    .font(.system(size: 64))
                    .foregroundStyle(AlpineTheme.alpineGreen)
                    .accessibilityLabel("Locked")

                Text("Alpine")
                    .font(.largeTitle.bold())

                Text("Authentication required")
                    .font(.subheadline)
                    .foregroundStyle(.secondary)
                    .accessibilityLabel("Authentication is required to unlock the app")

                if let viewModel {
                    VStack(spacing: 20) {
                        if viewModel.needsTOTP {
                            VStack(spacing: 8) {
                                Text("Enter TOTP Code")
                                    .font(.headline)
                                TextField("000000", text: Bindable(viewModel).totpCode)
                                    .keyboardType(.numberPad)
                                    .multilineTextAlignment(.center)
                                    .font(.system(.title, design: .monospaced))
                                    .frame(maxWidth: 200)
                                    .textFieldStyle(.roundedBorder)
                                    .accessibilityLabel("TOTP code entry")
                                    .accessibilityHint("Enter your 6-digit authentication code")
                            }
                        }

                        if viewModel.needsYubiKey {
                            Button {
                                Task { await viewModel.startYubiKeyScan() }
                            } label: {
                                Label("Tap YubiKey", systemImage: "wave.3.right")
                                    .frame(maxWidth: .infinity)
                            }
                            .buttonStyle(.bordered)
                            .disabled(viewModel.isYubiKeyScanning)
                            .accessibilityLabel("Tap YubiKey to authenticate")
                            .accessibilityHint("Starts scanning for a YubiKey device")
                        }

                        // Biometric authentication button
                        if authManager.biometricAvailable {
                            Button {
                                Task {
                                    biometricError = nil
                                    do {
                                        try await authManager.unlockWithBiometric()
                                    } catch {
                                        biometricError = error.localizedDescription
                                    }
                                }
                            } label: {
                                Label(
                                    biometricLabel,
                                    systemImage: biometricIcon
                                )
                                .frame(maxWidth: .infinity)
                            }
                            .buttonStyle(.bordered)
                            .tint(AlpineTheme.alpineGreen)
                            .accessibilityLabel(biometricLabel)
                            .accessibilityHint("Unlock the app using biometric authentication")
                        }

                        if let error = viewModel.error ?? biometricError {
                            Text(error)
                                .font(.caption)
                                .foregroundStyle(.red)
                                .multilineTextAlignment(.center)
                                .accessibilityLabel("Error: \(error)")
                        }

                        Button {
                            Task { await viewModel.unlock() }
                        } label: {
                            if viewModel.isAuthenticating {
                                ProgressView()
                                    .frame(maxWidth: .infinity)
                            } else {
                                Text("Unlock")
                                    .frame(maxWidth: .infinity)
                            }
                        }
                        .buttonStyle(.borderedProminent)
                        .tint(AlpineTheme.alpineGreen)
                        .disabled(viewModel.isAuthenticating)
                        .accessibilityLabel("Unlock")
                        .accessibilityHint("Submit credentials to unlock the app")
                    }
                    .padding(.horizontal, 40)
                }

                Spacer()
                Spacer()
            }
        }
        .onAppear {
            if viewModel == nil {
                viewModel = AuthViewModel(authManager: authManager)
            }
        }
    }

    private var biometricLabel: String {
        switch authManager.biometricService.biometricType {
        case .faceID: "Face ID"
        case .touchID: "Touch ID"
        case .none: "Biometric"
        }
    }

    private var biometricIcon: String {
        switch authManager.biometricService.biometricType {
        case .faceID: "faceid"
        case .touchID: "touchid"
        case .none: "lock.fill"
        }
    }
}
