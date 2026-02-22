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

    private func createRpcService() -> AlpineRpcService? {
        let tlsConfig = TlsConfig(
            enabled: settings.tlsEnabled,
            mode: settings.tlsMode,
            certFingerprint: settings.tlsCertFingerprint
        )
        guard let url = tlsConfig.buildURL(host: settings.host, port: settings.port) else {
            return nil
        }
        let apiKey = secureStorage.read(key: "apiKey") ?? ""
        let client = JsonRpcClient(baseURL: url, apiKey: apiKey, tlsConfig: tlsConfig)
        return AlpineRpcService(client: client)
    }

    func loadGroups() async {
        guard let rpc = createRpcService() else {
            message = "Invalid server configuration"
            return
        }
        isLoading = true
        do {
            let groupIds = try await rpc.listGroups()
            var infos: [GroupInfo] = []
            for id in groupIds {
                let info = try await rpc.getGroupInfo(groupId: id)
                infos.append(info)
            }
            groups = infos
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
        isLoading = false
    }

    func createGroup(name: String, description: String) async {
        guard let rpc = createRpcService() else { return }
        do {
            _ = try await rpc.createGroup(name: name, description: description)
            message = "Group created"
            await loadGroups()
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func deleteGroup(groupId: Int64) async {
        guard let rpc = createRpcService() else { return }
        do {
            _ = try await rpc.deleteGroup(groupId: groupId)
            message = "Group deleted"
            await loadGroups()
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func clearMessage() { message = nil }
}
