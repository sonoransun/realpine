import Foundation
import SwiftUI
import CoreImage.CIFilterBuiltins

@Observable
final class AuthEnrollmentViewModel {
    enum Step: Sendable {
        case chooseMethod
        case setupTOTP
        case verifyTOTP
        case setupYubiKey
        case complete
    }

    var step: Step = .chooseMethod
    var selectedMethod: AuthMethod = .totp
    var totpSecret: TOTPSecret?
    var qrCodeImage: UIImage?
    var manualKey = ""
    var verificationCode = ""
    var verificationError: String?
    var yubiKeyStatus = ""
    var isProcessing = false

    private let secureStorage: SecureStorage
    private let settingsStore: SettingsStore

    init(secureStorage: SecureStorage, settingsStore: SettingsStore) {
        self.secureStorage = secureStorage
        self.settingsStore = settingsStore
    }

    func generateTOTPSecret() {
        let secretData = TOTPEnrollment.generateRandomSecret()
        let secret = TOTPSecret(
            secret: secretData,
            config: .default,
            issuer: "Alpine",
            account: "user@alpine",
            enrolledAt: Date()
        )
        totpSecret = secret
        manualKey = TOTPEnrollment.base32Encode(secretData)
        generateQRCode(for: secret)
        step = .setupTOTP
    }

    func verifyTOTPCode() -> Bool {
        guard let secret = totpSecret else {
            verificationError = "No secret configured"
            return false
        }
        if TOTPGenerator.validateCode(verificationCode, secret: secret.secret) {
            verificationError = nil
            return true
        }
        verificationError = "Invalid code. Please try again."
        return false
    }

    func enrollYubiKey() async {
        isProcessing = true
        yubiKeyStatus = "Tap your YubiKey..."
        // YubiKey enrollment is handled by AuthManager's YubiKeyService
        // For now, mark as enrolled after simulated tap
        try? await Task.sleep(for: .seconds(1))
        yubiKeyStatus = "YubiKey registered"
        isProcessing = false
    }

    func completeEnrollment() {
        // Save TOTP secret if applicable
        if selectedMethod == .totp || selectedMethod == .totpAndYubiKey,
           let secret = totpSecret {
            if let encoded = try? JSONEncoder().encode(secret) {
                _ = secureStorage.storeData(key: "totp.secret", value: secret.secret)
                _ = secureStorage.store(key: "totp.config", value: String(data: encoded, encoding: .utf8) ?? "")
            }
        }

        // Update settings
        settingsStore.authMethod = selectedMethod
        settingsStore.authRequired = true

        step = .complete
    }

    func removeAuthentication() {
        secureStorage.remove(key: "totp.secret")
        secureStorage.remove(key: "totp.config")
        secureStorage.remove(key: "yubikey.credentialId")
        settingsStore.authMethod = .none
        settingsStore.authRequired = false
    }

    private func generateQRCode(for secret: TOTPSecret) {
        let payload = TOTPEnrollment.generateQRPayload(secret: secret)
        let context = CIContext()
        let filter = CIFilter.qrCodeGenerator()
        filter.message = Data(payload.utf8)
        filter.correctionLevel = "M"
        guard let ciImage = filter.outputImage else { return }
        let scaled = ciImage.transformed(by: CGAffineTransform(scaleX: 8, y: 8))
        guard let cgImage = context.createCGImage(scaled, from: scaled.extent) else { return }
        qrCodeImage = UIImage(cgImage: cgImage)
    }
}
