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

    private func createApiService() -> AlpineApiService {
        TransportProvider.createApiService(settings: settings, secureStorage: secureStorage)
    }

    func loadPeers() async {
        let api = createApiService()
        isLoading = true
        do {
            let peerIds = try await api.getAllPeers()
            var details: [PeerDetail] = []
            for id in peerIds {
                let detail = try await api.getPeer(peerId: id)
                details.append(detail)
            }
            peers = details
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
        isLoading = false
    }

    func addPeer(ipAddress: String, port: Int64) async {
        let api = createApiService()
        do {
            _ = try await api.addPeer(ipAddress: ipAddress, port: port)
            message = "Peer added successfully"
            await loadPeers()
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func pingPeer(peerId: Int64) async {
        let api = createApiService()
        do {
            let success = try await api.pingPeer(peerId: peerId)
            message = success ? "Ping successful" : "Ping failed"
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func activatePeer(peerId: Int64) async {
        let api = createApiService()
        do {
            _ = try await api.activatePeer(peerId: peerId)
            message = "Peer activated"
            await loadPeers()
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func deactivatePeer(peerId: Int64) async {
        let api = createApiService()
        do {
            _ = try await api.deactivatePeer(peerId: peerId)
            message = "Peer deactivated"
            await loadPeers()
            api.shutdown()
        } catch {
            message = ErrorMessages.userFriendly(from: error)
        }
    }

    func clearMessage() { message = nil }
}
