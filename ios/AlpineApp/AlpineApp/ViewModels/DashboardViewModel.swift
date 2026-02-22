import Foundation
import Observation

@Observable
final class DashboardViewModel {
    var serverStatus: ServerStatus?
    var peerCount: Int = 0
    var groupCount: Int = 0
    var defaultGroup: GroupInfo?
    var isLoading = false
    var error: String?
    var isConnected = false

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
    }

    func refresh() async {
        isLoading = true
        error = nil
        do {
            let tlsConfig = TlsConfig(
                enabled: settings.tlsEnabled,
                mode: settings.tlsMode,
                certFingerprint: settings.tlsCertFingerprint
            )
            guard let url = tlsConfig.buildURL(host: settings.host, port: settings.port) else {
                error = "Invalid server URL"
                isLoading = false
                return
            }
            let apiKey = secureStorage.read(key: "apiKey") ?? ""
            let client = JsonRpcClient(baseURL: url, apiKey: apiKey, tlsConfig: tlsConfig)
            let rpc = AlpineRpcService(client: client)

            serverStatus = try await rpc.getStatus()
            isConnected = true

            let peerIds = try await rpc.getAllPeers()
            peerCount = peerIds.count

            let groupIds = try await rpc.listGroups()
            groupCount = groupIds.count

            defaultGroup = try? await rpc.getDefaultGroupInfo()

            rpc.shutdown()
        } catch {
            self.error = ErrorMessages.userFriendly(from: error)
            isConnected = false
        }
        isLoading = false
    }
}
