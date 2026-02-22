import Foundation
import Observation

@Observable
final class PeersViewModel {
    var peers: [PeerDetail] = []
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

    func loadPeers() async {
        guard let rpc = createRpcService() else {
            message = "Invalid server configuration"
            return
        }
        isLoading = true
        do {
            let peerIds = try await rpc.getAllPeers()
            var details: [PeerDetail] = []
            for id in peerIds {
                let detail = try await rpc.getPeer(peerId: id)
                details.append(detail)
            }
            peers = details
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
        isLoading = false
    }

    func addPeer(ipAddress: String, port: Int64) async {
        guard let rpc = createRpcService() else { return }
        do {
            _ = try await rpc.addPeer(ipAddress: ipAddress, port: port)
            message = "Peer added successfully"
            await loadPeers()
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func pingPeer(peerId: Int64) async {
        guard let rpc = createRpcService() else { return }
        do {
            let success = try await rpc.pingPeer(peerId: peerId)
            message = success ? "Ping successful" : "Ping failed"
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func activatePeer(peerId: Int64) async {
        guard let rpc = createRpcService() else { return }
        do {
            _ = try await rpc.activatePeer(peerId: peerId)
            message = "Peer activated"
            await loadPeers()
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func deactivatePeer(peerId: Int64) async {
        guard let rpc = createRpcService() else { return }
        do {
            _ = try await rpc.deactivatePeer(peerId: peerId)
            message = "Peer deactivated"
            await loadPeers()
            rpc.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func clearMessage() { message = nil }
}
