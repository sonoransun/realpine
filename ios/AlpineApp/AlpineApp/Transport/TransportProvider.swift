import Foundation

/// Factory that creates transport and API service instances based on configuration.
enum TransportProvider {

    /// Creates a QueryTransport for query operations.
    static func createTransport(settings: SettingsStore, secureStorage: SecureStorage, authManager: AuthManager? = nil) -> QueryTransport {
        switch settings.transportMode {
        case .restBridge:
            let service = createApiService(settings: settings, secureStorage: secureStorage, authManager: authManager)
            return RestApiTransport(apiService: service)
        case .wifiBroadcast:
            return BroadcastTransport()
        }
    }

    /// Creates an AlpineApiService for full API access (all endpoints).
    static func createApiService(settings: SettingsStore, secureStorage: SecureStorage, authManager: AuthManager? = nil) -> AlpineApiService {
        let tlsConfig = TlsConfig(
            enabled: settings.tlsEnabled,
            mode: settings.tlsMode,
            certFingerprint: settings.tlsCertFingerprint
        )
        let baseURL = buildBaseURL(host: settings.host, port: settings.port, tlsConfig: tlsConfig)
        let apiKey = secureStorage.read(key: "apiKey") ?? ""
        let sessionToken = authManager?.currentAccessToken()
        let client = RestApiClient(baseURL: baseURL, apiKey: apiKey, sessionToken: sessionToken, tlsConfig: tlsConfig)
        return AlpineApiService(client: client)
    }

    /// Builds the base URL with /api/v1 path prefix.
    private static func buildBaseURL(host: String, port: String, tlsConfig: TlsConfig) -> URL {
        let scheme = tlsConfig.enabled ? "https" : "http"
        if let url = URL(string: "\(scheme)://\(host):\(port)/api/v1") {
            return url
        }
        // Fallback
        return URL(string: "http://\(host):\(port)/api/v1")!
    }
}
