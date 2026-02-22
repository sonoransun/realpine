import Foundation
import SwiftUI

@Observable
final class AuthViewModel {
    var totpCode = ""
    var isAuthenticating = false
    var error: String?
    var isYubiKeyScanning = false

    private let authManager: AuthManager

    init(authManager: AuthManager) {
        self.authManager = authManager
    }

    var needsTOTP: Bool {
        authManager.authMethod == .totp || authManager.authMethod == .totpAndYubiKey
    }

    var needsYubiKey: Bool {
        authManager.authMethod == .yubiKey || authManager.authMethod == .totpAndYubiKey
    }

    func unlock() async {
        isAuthenticating = true
        error = nil
        do {
            try await authManager.unlock(
                totpCode: needsTOTP ? totpCode : nil,
                yubiKeyOTP: nil
            )
        } catch {
            self.error = error.localizedDescription
        }
        isAuthenticating = false
        totpCode = ""
    }

    func startYubiKeyScan() async {
        isYubiKeyScanning = true
        error = nil
        do {
            try await authManager.unlock(totpCode: needsTOTP ? totpCode : nil, yubiKeyOTP: nil)
        } catch {
            self.error = error.localizedDescription
        }
        isYubiKeyScanning = false
    }
}
