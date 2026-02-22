import Foundation
import Observation

@Observable
final class GroupsViewModel {
    var groups: [GroupInfo] = []
    var isLoading = false
    var message: String?

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
    }

    private func createApiService() -> AlpineApiService {
        TransportProvider.createApiService(settings: settings, secureStorage: secureStorage)
    }

    func loadGroups() async {
        let api = createApiService()
        isLoading = true
        do {
            let groupIds = try await api.listGroups()
            var infos: [GroupInfo] = []
            for id in groupIds {
                let info = try await api.getGroupInfo(groupId: id)
                infos.append(info)
            }
            groups = infos
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
        isLoading = false
    }

    func createGroup(name: String, description: String) async {
        let api = createApiService()
        do {
            _ = try await api.createGroup(name: name, description: description)
            message = "Group created"
            await loadGroups()
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func deleteGroup(groupId: Int64) async {
        let api = createApiService()
        do {
            _ = try await api.deleteGroup(groupId: groupId)
            message = "Group deleted"
            await loadGroups()
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func clearMessage() { message = nil }
}
