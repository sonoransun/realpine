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

    func buildURL(host: String, port: String) -> URL? {
        let scheme = enabled ? "https" : "http"
        return URL(string: "\(scheme)://\(host):\(port)/rpc")
    }
}
