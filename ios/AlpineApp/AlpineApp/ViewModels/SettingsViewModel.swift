import Foundation
import Observation

@Observable
final class SettingsViewModel {
    var host: String
    var port: String
    var transportMode: TransportMode
    var tlsEnabled: Bool
    var tlsMode: TlsMode
    var tlsCertFingerprint: String
    var apiKey: String
    var connectionStatus: ConnectionStatus = .unknown
    var statusMessage: String = ""
    var discoveredBridges: [BridgeBeacon] { discoveryManager.discoveredBridges }
    var isScanning: Bool { discoveryManager.isScanning }
    var sharedDirectory: String
    var broadcastEnabled: Bool
    var indexedFileCount: Int { broadcastManager.indexedFileCount }
    var isBroadcastActive: Bool { broadcastManager.isActive }

    enum ConnectionStatus { case unknown, testing, connected, failed }

    private let settings: SettingsStore
    private let secureStorage: SecureStorage
    private let discoveryManager: WifiDiscoveryManager
    private let broadcastManager: BroadcastServiceManager
    private var testTask: Task<Void, Never>?

    init(
        settings: SettingsStore,
        secureStorage: SecureStorage,
        discoveryManager: WifiDiscoveryManager,
        broadcastManager: BroadcastServiceManager
    ) {
        self.settings = settings
        self.secureStorage = secureStorage
        self.discoveryManager = discoveryManager
        self.broadcastManager = broadcastManager
        // Load current values
        host = settings.host
        port = settings.port
        transportMode = settings.transportMode
        tlsEnabled = settings.tlsEnabled
        tlsMode = settings.tlsMode
        tlsCertFingerprint = settings.tlsCertFingerprint
        apiKey = secureStorage.read(key: "apiKey") ?? ""
        sharedDirectory = settings.sharedDirectory
        broadcastEnabled = settings.broadcastEnabled
    }

    func testConnection() {
        testTask?.cancel()
        connectionStatus = .testing
        statusMessage = "Testing..."
        testTask = Task {
            do {
                let tlsConfig = TlsConfig(
                    enabled: tlsEnabled,
                    mode: tlsMode,
                    certFingerprint: tlsCertFingerprint
                )
                let scheme = tlsConfig.enabled ? "https" : "http"
                guard let url = URL(string: "\(scheme)://\(host):\(port)/api/v1") else {
                    connectionStatus = .failed
                    statusMessage = "Invalid URL"
                    return
                }
                let client = RestApiClient(baseURL: url, apiKey: apiKey, tlsConfig: tlsConfig)
                let api = AlpineApiService(client: client)
                let status = try await api.getStatus()
                connectionStatus = .connected
                statusMessage = "Connected - v\(status.version)"
                api.shutdown()
            } catch {
                connectionStatus = .failed
                statusMessage = ErrorMessages.userFriendly(from: error)
            }
        }
    }

    func startDiscovery() { discoveryManager.start() }
    func stopDiscovery() { discoveryManager.stop() }

    func selectBridge(_ beacon: BridgeBeacon) {
        host = beacon.hostAddress
        port = String(beacon.restPort)
    }

    func startBroadcastServices() {
        broadcastManager.start(
            sharedDirectory: sharedDirectory,
            fileServerPort: settings.fileServerPort
        )
    }

    func stopBroadcastServices() { broadcastManager.stop() }

    func reindexContent() {
        broadcastManager.reindex(sharedDirectory: sharedDirectory)
    }

    func save() {
        settings.host = host
        settings.port = port
        settings.transportMode = transportMode
        settings.tlsEnabled = tlsEnabled
        settings.tlsMode = tlsMode
        settings.tlsCertFingerprint = tlsCertFingerprint
        settings.sharedDirectory = sharedDirectory
        settings.broadcastEnabled = broadcastEnabled
        _ = secureStorage.store(key: "apiKey", value: apiKey)
    }
}
