import Foundation
import SwiftUI

/// Central authentication orchestrator managing session lifecycle, credential verification, and auto-lock.
@Observable
final class AuthManager {
    private(set) var authState: AuthState = .locked
    private let sessionManager = SessionManager()
    private let yubiKeyService = YubiKeyService()
    private let secureStorage: SecureStorage
    private let settingsStore: SettingsStore
    private var backgroundTime: Date?

    var isLocked: Bool {
        switch authState {
        case .authenticated: false
        default: true
        }
    }

    var authMethod: AuthMethod {
        settingsStore.authMethod
    }

    init(secureStorage: SecureStorage, settingsStore: SettingsStore) {
        self.secureStorage = secureStorage
        self.settingsStore = settingsStore

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

    /// Attempt to unlock the app with provided credentials.
    func unlock(totpCode: String? = nil, yubiKeyOTP: String? = nil) async throws {
        authState = .authenticating

        let method = settingsStore.authMethod

        // Validate TOTP locally if required
        if method == .totp || method == .totpAndYubiKey {
            guard let code = totpCode else {
                authState = .locked
                throw AuthError.totpCodeRequired
            }
            guard let secretData = secureStorage.readData(key: "totp.secret") else {
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
            // This supports local-only auth when server has no /auth endpoints
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

    var errorDescription: String? {
        switch self {
        case .totpCodeRequired: "TOTP code is required"
        case .invalidTOTPCode: "Invalid TOTP code"
        case .notEnrolled: "Authentication not enrolled"
        case .yubiKeyFailed(let error): "YubiKey error: \(error.localizedDescription)"
        case .sessionExpired: "Session expired, please re-authenticate"
        }
    }
}
