import Foundation

enum InputValidator {
    /// Validates an IPv4 address (e.g. "192.168.1.1")
    static func isValidIP(_ ip: String) -> Bool {
        let pattern = #"^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$"#
        return ip.range(of: pattern, options: .regularExpression) != nil
    }

    /// Validates a port number string (1-65535)
    static func isValidPort(_ port: String) -> Bool {
        guard let portNum = Int(port) else {
            return false
        }
        return portNum >= 1 && portNum <= 65535
    }

    /// Validates a hostname (e.g. "server.local", "my-host")
    static func isValidHostname(_ host: String) -> Bool {
        let pattern = #"^[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)*$"#
        return host.range(of: pattern, options: .regularExpression) != nil
    }

    /// Validates a SHA-256 fingerprint (64 hex characters)
    static func isValidFingerprint(_ fp: String) -> Bool {
        let pattern = #"^[0-9a-fA-F]{64}$"#
        return fp.range(of: pattern, options: .regularExpression) != nil
    }

    /// Sanitizes a query string: trims whitespace and limits to 256 characters
    static func sanitizeQuery(_ query: String) -> String {
        let trimmed = query.trimmingCharacters(in: .whitespacesAndNewlines)
        if trimmed.count > 256 {
            return String(trimmed.prefix(256))
        }
        return trimmed
    }
}
