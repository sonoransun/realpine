import Foundation

/// Factory that creates the appropriate QueryTransport implementation
/// based on the current transport mode in settings.
enum TransportProvider {
    /// Creates a QueryTransport matching the user's configured transport mode.
    ///
    /// - Parameters:
    ///   - settings: The application settings store containing connection configuration
    ///   - secureStorage: Secure storage for reading API keys and credentials
    /// - Returns: A configured QueryTransport ready for use
    static func createTransport(settings: SettingsStore, secureStorage: SecureStorage) -> QueryTransport {
        switch settings.transportMode {
        case .restBridge:
            let tlsConfig = TlsConfig(
                enabled: settings.tlsEnabled,
                mode: settings.tlsMode,
                certFingerprint: settings.tlsCertFingerprint
            )

            guard let url = tlsConfig.buildURL(host: settings.host, port: settings.port) else {
                // Fallback URL when TLS config cannot build a valid URL
                let fallbackURL = URL(string: "http://\(settings.host):\(settings.port)/rpc")!
                let client = JsonRpcClient(baseURL: fallbackURL)
                return JsonRpcTransport(rpcService: AlpineRpcService(client: client))
            }

            let apiKey = secureStorage.read(key: "apiKey") ?? ""
            let client = JsonRpcClient(baseURL: url, apiKey: apiKey, tlsConfig: tlsConfig)
            return JsonRpcTransport(rpcService: AlpineRpcService(client: client))

        case .wifiBroadcast:
            return BroadcastTransport()
        }
    }
}
