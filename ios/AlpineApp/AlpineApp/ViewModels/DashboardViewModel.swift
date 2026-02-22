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
            let api = TransportProvider.createApiService(settings: settings, secureStorage: secureStorage)

            serverStatus = try await api.getStatus()
            isConnected = true

            let peerIds = try await api.getAllPeers()
            peerCount = peerIds.count

            let groupIds = try await api.listGroups()
            groupCount = groupIds.count

            defaultGroup = try? await api.getDefaultGroupInfo()

            api.shutdown()
        } catch {
            self.error = ErrorMessages.userFriendly(from: error)
            isConnected = false
        }
        isLoading = false
    }
}
