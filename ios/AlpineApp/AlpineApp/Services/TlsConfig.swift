import Foundation

enum TlsMode: String, Codable, CaseIterable, Sendable {
    case systemCA = "SYSTEM_CA"
    case pinned = "PINNED"
    case trustAll = "TRUST_ALL"
}

struct TlsConfig: Sendable {
    var enabled: Bool = false
    var mode: TlsMode = .systemCA
    var certFingerprint: String = ""
    var backupFingerprints: [String] = []

    /// All fingerprints to check (primary + backups). Filters out empty strings.
    var allFingerprints: [String] {
        ([certFingerprint] + backupFingerprints).filter { !$0.isEmpty }
    }

    func buildURL(host: String, port: String) -> URL? {
        let scheme = enabled ? "https" : "http"
        return URL(string: "\(scheme)://\(host):\(port)/api/v1")
    }
}
