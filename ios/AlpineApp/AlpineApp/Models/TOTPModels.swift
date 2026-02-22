import Foundation

/// Configuration for TOTP code generation per RFC 6238.
struct TOTPConfig: Codable, Sendable {
    let period: Int
    let digits: Int
    let algorithm: TOTPAlgorithm

    static let `default` = TOTPConfig(period: 30, digits: 6, algorithm: .sha1)
}

/// HMAC algorithm used for TOTP generation.
enum TOTPAlgorithm: String, Codable, Sendable {
    case sha1
    case sha256
    case sha512
}

/// An enrolled TOTP secret with metadata.
struct TOTPSecret: Codable, Sendable {
    let secret: Data
    let config: TOTPConfig
    let issuer: String
    let account: String
    let enrolledAt: Date
}
