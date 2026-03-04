import Foundation

/// Authentication method options for app unlock.
enum AuthMethod: String, Codable, Sendable, CaseIterable {
    case none
    case totp
    case yubiKey
    case biometric
    case totpAndYubiKey
    case secureEnclave
}

/// Current authentication state of the app.
enum AuthState: Sendable {
    case locked
    case authenticating
    case authenticated(session: SessionToken)
    case expired
}

/// Represents a server-issued session token with expiry tracking.
struct SessionToken: Codable, Sendable {
    let accessToken: String
    let refreshToken: String?
    let expiresAt: Date
    let issuedAt: Date

    var isExpired: Bool { Date() >= expiresAt }

    var needsRefresh: Bool {
        let lifetime = expiresAt.timeIntervalSince(issuedAt)
        return Date().timeIntervalSince(issuedAt) >= lifetime * 0.8
    }
}

/// Server-generated challenge for YubiKey authentication.
struct AuthChallenge: Codable, Sendable {
    let challengeId: String
    let nonce: String
    let timestamp: Date
}

/// Authentication request sent to the server.
struct AuthRequest: Codable, Sendable {
    let method: String
    let totpCode: String?
    let yubiKeyOTP: String?
    let challengeId: String?
}

/// Authentication response from the server.
struct AuthResponse: Codable, Sendable {
    let accessToken: String
    let refreshToken: String?
    let expiresIn: Int
}

/// A stored user credential reference.
struct UserCredential: Codable, Sendable {
    let id: UUID
    let method: AuthMethod
    let createdAt: Date
    let label: String
}

/// Request to register a device's Secure Enclave public key with the server.
struct DeviceEnrollmentRequest: Codable, Sendable {
    let publicKey: String
    let deviceName: String
    let biometricType: String
}

/// Server response to device enrollment.
struct DeviceEnrollmentResponse: Codable, Sendable {
    let deviceId: String
    let enrolledAt: Date
}

/// Request to verify a challenge signature for Secure Enclave authentication.
struct ChallengeSignatureRequest: Codable, Sendable {
    let challengeId: String
    let signature: String
    let publicKey: String
}
