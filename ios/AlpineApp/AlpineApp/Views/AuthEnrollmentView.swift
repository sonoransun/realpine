import SwiftUI

struct AuthEnrollmentView: View {
    @State private var viewModel: AuthEnrollmentViewModel
    @Environment(\.dismiss) private var dismiss

    init(secureStorage: SecureStorage, settingsStore: SettingsStore) {
        _viewModel = State(wrappedValue: AuthEnrollmentViewModel(
            secureStorage: secureStorage,
            settingsStore: settingsStore
        ))
    }

    var body: some View {
        NavigationStack {
            Group {
                switch viewModel.step {
                case .chooseMethod:
                    chooseMethodView
                case .setupTOTP:
                    setupTOTPView
                case .verifyTOTP:
                    verifyTOTPView
                case .setupYubiKey:
                    setupYubiKeyView
                case .complete:
                    completeView
                }
            }
            .navigationTitle("Set Up Authentication")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    if viewModel.step != .complete {
                        Button("Cancel") { dismiss() }
                    }
                }
            }
        }
    }

    // MARK: - Choose Method

    private var chooseMethodView: some View {
        VStack(spacing: 24) {
            Text("Choose Authentication Method")
                .font(.headline)

            Picker("Method", selection: $viewModel.selectedMethod) {
                Text("TOTP").tag(AuthMethod.totp)
                Text("YubiKey").tag(AuthMethod.yubiKey)
                Text("Both").tag(AuthMethod.totpAndYubiKey)
            }
            .pickerStyle(.segmented)
            .padding(.horizontal)

            VStack(alignment: .leading, spacing: 8) {
                switch viewModel.selectedMethod {
                case .totp:
                    Label("Time-based one-time passwords", systemImage: "clock")
                    Label("Works with any authenticator app", systemImage: "iphone")
                case .yubiKey:
                    Label("Hardware security key", systemImage: "key.fill")
                    Label("NFC or Lightning/USB-C", systemImage: "wave.3.right")
                case .totpAndYubiKey:
                    Label("Maximum security: both factors required", systemImage: "lock.shield")
                default:
                    EmptyView()
                }
            }
            .font(.subheadline)
            .foregroundStyle(.secondary)
            .padding(.horizontal)

            Spacer()

            Button("Continue") {
                switch viewModel.selectedMethod {
                case .totp:
                    viewModel.generateTOTPSecret()
                case .yubiKey:
                    viewModel.step = .setupYubiKey
                case .totpAndYubiKey:
                    viewModel.generateTOTPSecret()
                default:
                    break
                }
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
        }
        .padding()
    }

    // MARK: - Setup TOTP

    private var setupTOTPView: some View {
        ScrollView {
            VStack(spacing: 24) {
                Text("Scan QR Code")
                    .font(.headline)

                Text("Scan this code with your authenticator app (Google Authenticator, Authy, etc.)")
                    .font(.subheadline)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.center)

                if let image = viewModel.qrCodeImage {
                    Image(uiImage: image)
                        .interpolation(.none)
                        .resizable()
                        .scaledToFit()
                        .frame(width: 200, height: 200)
                        .background(Color.white)
                        .cornerRadius(12)
                }

                VStack(spacing: 4) {
                    Text("Manual Entry Key")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    Text(viewModel.manualKey)
                        .font(.system(.body, design: .monospaced))
                        .textSelection(.enabled)
                }

                Button("Next: Verify Code") {
                    viewModel.step = .verifyTOTP
                }
                .buttonStyle(.borderedProminent)
                .tint(AlpineTheme.alpineGreen)
            }
            .padding()
        }
    }

    // MARK: - Verify TOTP

    private var verifyTOTPView: some View {
        VStack(spacing: 24) {
            Text("Verify Setup")
                .font(.headline)

            Text("Enter the 6-digit code from your authenticator app to confirm setup.")
                .font(.subheadline)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)

            TextField("000000", text: $viewModel.verificationCode)
                .keyboardType(.numberPad)
                .multilineTextAlignment(.center)
                .font(.system(.title, design: .monospaced))
                .frame(maxWidth: 200)
                .textFieldStyle(.roundedBorder)

            if let error = viewModel.verificationError {
                Text(error)
                    .font(.caption)
                    .foregroundStyle(.red)
            }

            Spacer()

            Button("Verify & Enable") {
                if viewModel.verifyTOTPCode() {
                    if viewModel.selectedMethod == .totpAndYubiKey {
                        viewModel.step = .setupYubiKey
                    } else {
                        viewModel.completeEnrollment()
                    }
                }
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
            .disabled(viewModel.verificationCode.count < 6)
        }
        .padding()
    }

    // MARK: - Setup YubiKey

    private var setupYubiKeyView: some View {
        VStack(spacing: 24) {
            Text("Register YubiKey")
                .font(.headline)

            Image(systemName: "wave.3.right.circle.fill")
                .font(.system(size: 80))
                .foregroundStyle(AlpineTheme.alpineGreen)

            Text(viewModel.yubiKeyStatus.isEmpty ? "Tap your YubiKey to the back of your device, or connect via Lightning/USB-C." : viewModel.yubiKeyStatus)
                .font(.subheadline)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)

            if viewModel.isProcessing {
                ProgressView()
            }

            Spacer()

            Button("Register YubiKey") {
                Task { await viewModel.enrollYubiKey() }
            }
            .buttonStyle(.bordered)
            .disabled(viewModel.isProcessing)

            Button("Complete Setup") {
                viewModel.completeEnrollment()
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
            .disabled(viewModel.yubiKeyStatus.isEmpty)
        }
        .padding()
    }

    // MARK: - Complete

    private var completeView: some View {
        VStack(spacing: 24) {
            Spacer()

            Image(systemName: "checkmark.shield.fill")
                .font(.system(size: 64))
                .foregroundStyle(.green)

            Text("Authentication Enabled")
                .font(.title2.bold())

            Text("Your app is now protected.")
                .foregroundStyle(.secondary)

            Spacer()

            Button("Done") {
                dismiss()
            }
            .buttonStyle(.borderedProminent)
            .tint(AlpineTheme.alpineGreen)
        }
        .padding()
    }
}
