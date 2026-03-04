import Foundation
import SwiftUI
import UIKit

/// Central authentication orchestrator managing session lifecycle, credential verification, and auto-lock.
@Observable
final class AuthManager {
    private(set) var authState: AuthState = .locked
    private let sessionManager = SessionManager()
    private let yubiKeyService = YubiKeyService()
    let biometricService = BiometricAuthService()
    private let secureStorage: SecureStorage
    private let settingsStore: SettingsStore
    private var backgroundTime: Date?
    private let enclaveManager = SecureEnclaveManager()

    var isLocked: Bool {
        switch authState {
        case .authenticated: false
        default: true
        }
    }

    var authMethod: AuthMethod {
        settingsStore.authMethod
    }

    var biometricAvailable: Bool { biometricService.isAvailable }

    init(secureStorage: SecureStorage, settingsStore: SettingsStore) {
        self.secureStorage = secureStorage
        self.settingsStore = settingsStore

        biometricService.checkAvailability()

        // Check for biometric enrollment changes
        if settingsStore.authRequired {
            let storedState = secureStorage.readData(key: "biometric.domainState")
            if biometricService.hasEnrollmentChanged(storedState: storedState) {
                // Biometric enrollment changed — clear credentials for security
                secureStorage.remove(key: "session.accessToken")
                secureStorage.remove(key: "session.refreshToken")
                secureStorage.removeBiometricProtected(key: "apiKey")
                secureStorage.removeBiometricProtected(key: "session.accessToken")
                secureStorage.removeBiometricProtected(key: "session.refreshToken")
                // Note: Secure Enclave keys with .biometryCurrentSet are auto-invalidated by iOS
            }
        }

        // If auth not required, auto-authenticate
        if !settingsStore.authRequired {
            authState = .authenticated(session: SessionToken(
                accessToken: "",
                refreshToken: nil,
                expiresAt: .distantFuture,
                issuedAt: Date()
            ))
        }
    }

    /// Attempt to unlock the app using biometric authentication (Face ID / Touch ID).
    func unlockWithBiometric() async throws {
        authState = .authenticating

        // If Secure Enclave is enabled and device has a key pair, use challenge-response
        if settingsStore.secureEnclaveEnabled && enclaveManager.hasKeyPair {
            try await unlockWithSecureEnclave()
            return
        }

        // Use biometric-protected Keychain: the system biometric prompt is triggered
        // by the Keychain read itself (hardware-enforced, not app-logic)
        do {
            let apiKey = try await secureStorage.readBiometricProtected(key: "apiKey")

            // Attempt server session auth
            do {
                let request = AuthRequest(
                    method: AuthMethod.biometric.rawValue,
                    totpCode: nil,
                    yubiKeyOTP: nil,
                    challengeId: nil
                )
                let baseURL = buildBaseURL()
                let session = try await sessionManager.authenticate(request: request, baseURL: baseURL)
                _ = secureStorage.storeBiometricProtected(key: "session.accessToken", value: session.accessToken)
                if let refresh = session.refreshToken {
                    _ = secureStorage.storeBiometricProtected(key: "session.refreshToken", value: refresh)
                }
                storeBiometricDomainState()
                authState = .authenticated(session: session)
            } catch {
                // Server not available — authenticate locally
                let localSession = SessionToken(
                    accessToken: apiKey ?? "",
                    refreshToken: nil,
                    expiresAt: Date().addingTimeInterval(3600),
                    issuedAt: Date()
                )
                storeBiometricDomainState()
                authState = .authenticated(session: localSession)
            }
        } catch SecureStorageError.biometricAuthFailed {
            authState = .locked
            throw AuthError.biometricFailed
        } catch SecureStorageError.itemNotFound {
            // No biometric-protected API key yet — fall back to legacy biometric auth
            // and migrate the key on success
            let success = try await biometricService.authenticate(reason: "Unlock Alpine")
            guard success else {
                authState = .locked
                throw AuthError.biometricFailed
            }

            // Migrate API key to biometric-protected storage
            if let apiKey = secureStorage.read(key: "apiKey"), !apiKey.isEmpty {
                _ = secureStorage.storeBiometricProtected(key: "apiKey", value: apiKey)
            }

            let localSession = SessionToken(
                accessToken: secureStorage.read(key: "apiKey") ?? "",
                refreshToken: nil,
                expiresAt: Date().addingTimeInterval(3600),
                issuedAt: Date()
            )
            storeBiometricDomainState()
            authState = .authenticated(session: localSession)
        }
    }

    /// Authenticate using Secure Enclave challenge-response (biometric is triggered by the signing operation).
    private func unlockWithSecureEnclave() async throws {
        guard SecureEnclaveManager.isAvailable else {
            authState = .locked
            throw AuthError.secureEnclaveUnavailable
        }

        let baseURL = buildBaseURL()

        // 1. Request challenge from server
        let challenge: AuthChallenge
        do {
            challenge = try await sessionManager.requestChallenge(baseURL: baseURL)
        } catch {
            authState = .locked
            throw AuthError.challengeSigningFailed
        }

        // 2. Sign the challenge nonce with Secure Enclave key (triggers biometric)
        let nonceData = Data(challenge.nonce.utf8)
        let signature: Data
        do {
            signature = try await enclaveManager.sign(data: nonceData)
        } catch {
            authState = .locked
            throw AuthError.challengeSigningFailed
        }

        // 3. Get public key for verification
        let publicKeyData: Data
        do {
            publicKeyData = try enclaveManager.publicKeyData()
        } catch {
            authState = .locked
            throw AuthError.challengeSigningFailed
        }

        // 4. Submit signature to server for verification
        let verifyRequest = ChallengeSignatureRequest(
            challengeId: challenge.challengeId,
            signature: signature.base64EncodedString(),
            publicKey: publicKeyData.base64EncodedString()
        )

        do {
            let session = try await sessionManager.verifyChallenge(request: verifyRequest, baseURL: baseURL)
            _ = secureStorage.storeBiometricProtected(key: "session.accessToken", value: session.accessToken)
            if let refresh = session.refreshToken {
                _ = secureStorage.storeBiometricProtected(key: "session.refreshToken", value: refresh)
            }
            storeBiometricDomainState()
            authState = .authenticated(session: session)
        } catch {
            authState = .locked
            throw AuthError.challengeSigningFailed
        }
    }

    /// Enroll the current device's Secure Enclave key pair with the server.
    func enrollDevice() async throws {
        guard SecureEnclaveManager.isAvailable else {
            throw AuthError.secureEnclaveUnavailable
        }

        // Generate a new key pair (with biometric protection)
        _ = try enclaveManager.generateKeyPair(biometricProtected: true)

        let publicKeyData = try enclaveManager.publicKeyData()

        let biometricType: String
        switch biometricService.biometricType {
        case .faceID: biometricType = "faceID"
        case .touchID: biometricType = "touchID"
        case .none: biometricType = "none"
        }

        let enrollRequest = DeviceEnrollmentRequest(
            publicKey: publicKeyData.base64EncodedString(),
            deviceName: UIDevice.current.name,
            biometricType: biometricType
        )

        let baseURL = buildBaseURL()
        try await sessionManager.enrollDevice(request: enrollRequest, baseURL: baseURL)

        // Store enrollment status
        _ = secureStorage.storeBiometricProtected(key: "device.enrolled", value: "true")
    }

    /// Attempt to unlock the app with provided credentials.
    func unlock(totpCode: String? = nil, yubiKeyOTP: String? = nil) async throws {
        authState = .authenticating

        let method = settingsStore.authMethod

        // Handle biometric method via dedicated flow
        if method == .biometric {
            try await unlockWithBiometric()
            return
        }

        if method == .secureEnclave {
            try await unlockWithSecureEnclave()
            return
        }

        // Validate TOTP locally if required
        if method == .totp || method == .totpAndYubiKey {
            guard let code = totpCode else {
                authState = .locked
                throw AuthError.totpCodeRequired
            }
            let secretData: Data?
            if settingsStore.requireBiometricForData {
                secretData = try await secureStorage.readBiometricProtectedData(key: "totp.secret")
            } else {
                secretData = secureStorage.readData(key: "totp.secret")
            }
            guard let secretData else {
                authState = .locked
                throw AuthError.notEnrolled
            }
            guard TOTPGenerator.validateCode(code, secret: secretData) else {
                authState = .locked
                throw AuthError.invalidTOTPCode
            }
        }

        // Validate YubiKey if required
        var yubiOTP = yubiKeyOTP
        if method == .yubiKey || method == .totpAndYubiKey {
            if yubiOTP == nil {
                do {
                    yubiOTP = try await yubiKeyService.requestOTP()
                } catch {
                    authState = .locked
                    throw AuthError.yubiKeyFailed(error)
                }
            }
        }

        // Build auth request and authenticate with server
        let request = AuthRequest(
            method: method.rawValue,
            totpCode: totpCode,
            yubiKeyOTP: yubiOTP,
            challengeId: nil
        )

        do {
            let baseURL = buildBaseURL()
            let session = try await sessionManager.authenticate(request: request, baseURL: baseURL)
            _ = secureStorage.store(key: "session.accessToken", value: session.accessToken)
            if let refresh = session.refreshToken {
                _ = secureStorage.store(key: "session.refreshToken", value: refresh)
            }
            authState = .authenticated(session: session)
        } catch {
            // If server auth fails but local validation passed, still authenticate locally
            let localSession = SessionToken(
                accessToken: secureStorage.read(key: "apiKey") ?? "",
                refreshToken: nil,
                expiresAt: Date().addingTimeInterval(3600),
                issuedAt: Date()
            )
            authState = .authenticated(session: localSession)
        }
    }

    /// Lock the app.
    func lock() {
        authState = .locked
    }

    /// Logout: invalidate server session and clear stored credentials.
    func logout() async {
        if let token = currentAccessToken() {
            try? await sessionManager.invalidateSession(accessToken: token, baseURL: buildBaseURL())
        }
        secureStorage.remove(key: "session.accessToken")
        secureStorage.remove(key: "session.refreshToken")
        secureStorage.removeBiometricProtected(key: "session.accessToken")
        secureStorage.removeBiometricProtected(key: "session.refreshToken")
        secureStorage.removeBiometricProtected(key: "apiKey")
        secureStorage.removeBiometricProtected(key: "device.enrolled")
        secureStorage.remove(key: "biometric.domainState")
        enclaveManager.deleteKeyPair()
        authState = .locked
    }

    /// Get the current valid access token, or nil if expired/unavailable.
    func currentAccessToken() -> String? {
        switch authState {
        case .authenticated(let session):
            return session.isExpired ? nil : session.accessToken
        default:
            return nil
        }
    }

    /// Check auto-lock based on scene phase changes.
    func checkAutoLock(phase: ScenePhase) {
        switch phase {
        case .background:
            backgroundTime = Date()
        case .active:
            guard settingsStore.authRequired,
                  settingsStore.autoLockTimeout >= 0,
                  let bgTime = backgroundTime else {
                backgroundTime = nil
                return
            }
            let elapsed = Date().timeIntervalSince(bgTime)
            if elapsed >= Double(settingsStore.autoLockTimeout) {
                lock()
            }
            backgroundTime = nil
        default:
            break
        }
    }

    /// Save the current biometric domain state for future enrollment change detection.
    private func storeBiometricDomainState() {
        if let state = biometricService.currentBiometricDomainState() {
            _ = secureStorage.storeData(key: "biometric.domainState", value: state)
        }
    }

    private func buildBaseURL() -> URL {
        let scheme = settingsStore.tlsEnabled ? "https" : "http"
        return URL(string: "\(scheme)://\(settingsStore.host):\(settingsStore.port)/api/v1")
            ?? URL(string: "http://localhost:8080/api/v1")!
    }
}

/// Authentication-specific errors.
enum AuthError: Error, LocalizedError {
    case totpCodeRequired
    case invalidTOTPCode
    case notEnrolled
    case yubiKeyFailed(Error)
    case sessionExpired
    case biometricFailed
    case secureEnclaveUnavailable
    case enrollmentChanged
    case challengeSigningFailed
    case deviceNotEnrolled

    var errorDescription: String? {
        switch self {
        case .totpCodeRequired: "TOTP code is required"
        case .invalidTOTPCode: "Invalid TOTP code"
        case .notEnrolled: "Authentication not enrolled"
        case .yubiKeyFailed(let error): "YubiKey error: \(error.localizedDescription)"
        case .sessionExpired: "Session expired, please re-authenticate"
        case .biometricFailed: "Biometric authentication failed"
        case .secureEnclaveUnavailable: "Secure Enclave is not available on this device"
        case .enrollmentChanged: "Biometric enrollment changed, please re-authenticate"
        case .challengeSigningFailed: "Failed to sign authentication challenge"
        case .deviceNotEnrolled: "Device is not enrolled for Secure Enclave authentication"
        }
    }
}
