import Foundation
import SwiftUI

@Observable
final class SettingsStore {
    private static let defaults = UserDefaults.standard
    private static let secureStorage = SecureStorage()

    private static let keyPrefix = "alpine."

    var host: String {
        didSet { Self.defaults.set(host, forKey: Self.keyPrefix + "host") }
    }

    var port: String {
        didSet { Self.defaults.set(port, forKey: Self.keyPrefix + "port") }
    }

    var transportMode: TransportMode {
        didSet { Self.defaults.set(transportMode.rawValue, forKey: Self.keyPrefix + "transportMode") }
    }

    var tlsEnabled: Bool {
        didSet { Self.defaults.set(tlsEnabled, forKey: Self.keyPrefix + "tlsEnabled") }
    }

    var tlsMode: TlsMode {
        didSet { Self.defaults.set(tlsMode.rawValue, forKey: Self.keyPrefix + "tlsMode") }
    }

    var tlsCertFingerprint: String {
        didSet { Self.defaults.set(tlsCertFingerprint, forKey: Self.keyPrefix + "tlsCertFingerprint") }
    }

    var apiKey: String {
        didSet { _ = Self.secureStorage.store(key: "apiKey", value: apiKey) }
    }

    var sharedDirectory: String {
        didSet { Self.defaults.set(sharedDirectory, forKey: Self.keyPrefix + "sharedDirectory") }
    }

    var broadcastEnabled: Bool {
        didSet { Self.defaults.set(broadcastEnabled, forKey: Self.keyPrefix + "broadcastEnabled") }
    }

    var fileServerPort: Int {
        didSet { Self.defaults.set(fileServerPort, forKey: Self.keyPrefix + "fileServerPort") }
    }

    init() {
        let defaults = Self.defaults
        let prefix = Self.keyPrefix

        self.host = defaults.string(forKey: prefix + "host") ?? "localhost"
        self.port = defaults.string(forKey: prefix + "port") ?? "8080"

        if let modeRaw = defaults.string(forKey: prefix + "transportMode"),
           let mode = TransportMode(rawValue: modeRaw) {
            self.transportMode = mode
        } else {
            self.transportMode = .restBridge
        }

        self.tlsEnabled = defaults.bool(forKey: prefix + "tlsEnabled")

        if let tlsModeRaw = defaults.string(forKey: prefix + "tlsMode"),
           let mode = TlsMode(rawValue: tlsModeRaw) {
            self.tlsMode = mode
        } else {
            self.tlsMode = .systemCA
        }

        self.tlsCertFingerprint = defaults.string(forKey: prefix + "tlsCertFingerprint") ?? ""
        self.apiKey = Self.secureStorage.read(key: "apiKey") ?? ""
        self.sharedDirectory = defaults.string(forKey: prefix + "sharedDirectory") ?? ""
        self.broadcastEnabled = defaults.bool(forKey: prefix + "broadcastEnabled")

        let storedPort = defaults.integer(forKey: prefix + "fileServerPort")
        self.fileServerPort = storedPort != 0 ? storedPort : 8091
    }
}
