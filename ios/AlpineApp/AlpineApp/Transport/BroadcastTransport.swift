import Foundation
import Network

/// UDP broadcast-based transport for peer-to-peer Alpine queries on the local network.
/// Sends query broadcasts on port 8090 and listens for responses from peers.
/// Conforms to QueryTransport with @unchecked Sendable since internal state
/// is protected by NSLock.
final class BroadcastTransport: QueryTransport, @unchecked Sendable {
    private let broadcastPort: UInt16 = 8090
    private let queryTimeout: TimeInterval = 30.0
    private let senderId: String
    private var activeQueries: [Int64: QueryState] = [:]
    private let lock = NSLock()
    private var listener: NWListener?
    private let queue = DispatchQueue(label: "com.alpine.broadcast.transport", qos: .utility)

    /// Internal state tracking for an active broadcast query.
    private struct QueryState {
        var inProgress: Bool
        var totalPeers: Set<String>
        var totalHits: Int64
        var results: [PeerResources]
        var autoHaltLimit: Int64
        var timeoutTask: Task<Void, Never>?
    }

    init() {
        senderId = String(UUID().uuidString.prefix(8)).lowercased()
        startListener()
    }

    // MARK: - QueryTransport

    func startQuery(_ request: QueryRequest) async throws -> QueryResponse {
        let queryId = Int64(Date().timeIntervalSince1970 * 1000)

        let state = QueryState(
            inProgress: true,
            totalPeers: [],
            totalHits: 0,
            results: [],
            autoHaltLimit: request.autoHaltLimit,
            timeoutTask: nil
        )

        lock.lock()
        activeQueries[queryId] = state
        lock.unlock()

        // Start timeout task
        let timeoutTask = Task { [weak self] in
            try? await Task.sleep(for: .seconds(self?.queryTimeout ?? 30.0))
            guard !Task.isCancelled else { return }
            self?.markQueryComplete(queryId)
        }

        lock.lock()
        activeQueries[queryId]?.timeoutTask = timeoutTask
        lock.unlock()

        // Build and send the broadcast query
        let localAddress = NetworkUtility.localIPAddress() ?? "0.0.0.0"
        let queryMessage: [String: Any] = [
            "type": "alpine_query",
            "queryId": queryId,
            "senderId": senderId,
            "senderAddress": localAddress,
            "senderPort": Int(broadcastPort),
            "queryString": request.queryString,
            "groupName": request.groupName,
            "autoHaltLimit": request.autoHaltLimit,
            "peerDescMax": request.peerDescMax,
            "timestamp": Int64(Date().timeIntervalSince1970 * 1000)
        ]

        try sendBroadcast(queryMessage)

        return QueryResponse(queryId: queryId)
    }

    func getQueryStatus(_ queryId: Int64) async throws -> QueryStatusResponse {
        lock.lock()
        let state = activeQueries[queryId]
        lock.unlock()

        guard let state else {
            return QueryStatusResponse(
                inProgress: false,
                totalPeers: 0,
                peersQueried: 0,
                numPeerResponses: 0,
                totalHits: 0
            )
        }

        let peerCount = Int64(state.totalPeers.count)
        return QueryStatusResponse(
            inProgress: state.inProgress,
            totalPeers: peerCount,
            peersQueried: peerCount,
            numPeerResponses: peerCount,
            totalHits: state.totalHits
        )
    }

    func getQueryResults(_ queryId: Int64) async throws -> QueryResultsResponse {
        lock.lock()
        let state = activeQueries[queryId]
        lock.unlock()

        return QueryResultsResponse(peers: state?.results ?? [])
    }

    func cancelQuery(_ queryId: Int64) async throws {
        // Send cancel broadcast
        let cancelMessage: [String: Any] = [
            "type": "alpine_cancel",
            "queryId": queryId,
            "senderId": senderId
        ]

        try? sendBroadcast(cancelMessage)

        markQueryComplete(queryId)
    }

    func shutdown() {
        lock.lock()
        let queryIds = Array(activeQueries.keys)
        lock.unlock()

        for queryId in queryIds {
            markQueryComplete(queryId)
        }

        listener?.cancel()
        listener = nil
    }

    // MARK: - Listener

    /// Starts a UDP listener on the broadcast port to receive query responses.
    private func startListener() {
        do {
            let params = NWParameters.udp
            params.allowLocalEndpointReuse = true
            params.requiredInterfaceType = .wifi

            listener = try NWListener(using: params, on: NWEndpoint.Port(integerLiteral: broadcastPort))
        } catch {
            print("[BroadcastTransport] Failed to create listener: \(error)")
            return
        }

        listener?.newConnectionHandler = { [weak self] connection in
            self?.handleIncomingConnection(connection)
        }

        let port = broadcastPort
        listener?.stateUpdateHandler = { state in
            switch state {
            case .ready:
                print("[BroadcastTransport] Listening for responses on port \(port)")
            case .failed(let error):
                print("[BroadcastTransport] Listener failed: \(error)")
            default:
                break
            }
        }

        listener?.start(queue: queue)
    }

    private func handleIncomingConnection(_ connection: NWConnection) {
        connection.start(queue: queue)

        connection.receiveMessage { [weak self] data, _, _, error in
            defer { connection.cancel() }

            if let error {
                print("[BroadcastTransport] Receive error: \(error)")
                return
            }

            guard let data else { return }
            self?.handleResponseData(data)
        }
    }

    // MARK: - Response Handling

    private func handleResponseData(_ data: Data) {
        guard let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any] else {
            return
        }

        guard let type = json["type"] as? String, type == "alpine_response" else {
            return
        }

        guard let queryId = json["queryId"] as? Int64 else {
            return
        }

        let responderId = json["responderId"] as? String ?? "unknown"

        // Parse resources from the response
        var resources: [ResourceDesc] = []
        if let resourcesArray = json["resources"] as? [[String: Any]] {
            for (index, resDict) in resourcesArray.enumerated() {
                let resourceId = resDict["resourceId"] as? Int64 ?? Int64(index + 1)
                let size = resDict["size"] as? Int64 ?? 0
                let description = resDict["description"] as? String ?? ""
                let locators: [String]
                if let locatorArray = resDict["locators"] as? [String] {
                    locators = locatorArray
                } else {
                    locators = []
                }

                resources.append(ResourceDesc(
                    resourceId: resourceId,
                    size: size,
                    locators: locators,
                    description: description
                ))
            }
        }

        lock.lock()
        guard var state = activeQueries[queryId], state.inProgress else {
            lock.unlock()
            return
        }

        // Track unique responder
        state.totalPeers.insert(responderId)
        let peerId = Int64(abs(responderId.hashValue) % Int.max)

        // Add peer resources
        let peerResources = PeerResources(peerId: peerId, resources: resources)
        state.results.append(peerResources)
        state.totalHits += Int64(resources.count)

        // Check auto-halt limit
        if state.totalHits >= state.autoHaltLimit {
            state.inProgress = false
            state.timeoutTask?.cancel()
        }

        activeQueries[queryId] = state
        lock.unlock()
    }

    // MARK: - Broadcast Sending

    private func sendBroadcast(_ message: [String: Any]) throws {
        guard let data = try? JSONSerialization.data(withJSONObject: message) else {
            throw BroadcastError.serializationFailed
        }

        let broadcastEndpoint = NWEndpoint.hostPort(
            host: NWEndpoint.Host("255.255.255.255"),
            port: NWEndpoint.Port(integerLiteral: broadcastPort)
        )

        let params = NWParameters.udp
        params.allowLocalEndpointReuse = true
        params.requiredInterfaceType = .wifi

        // Enable broadcast on the UDP socket
        if let ipOptions = params.defaultProtocolStack.internetProtocol as? NWProtocolIP.Options {
            ipOptions.disableMulticastLoopback = false
        }

        let connection = NWConnection(to: broadcastEndpoint, using: params)
        connection.start(queue: queue)

        connection.send(content: data, completion: .contentProcessed { error in
            if let error {
                print("[BroadcastTransport] Send error: \(error)")
            }
            // Allow a brief moment for the send to complete, then cancel
            DispatchQueue.global().asyncAfter(deadline: .now() + 0.5) {
                connection.cancel()
            }
        })
    }

    // MARK: - Query Lifecycle

    private func markQueryComplete(_ queryId: Int64) {
        lock.lock()
        activeQueries[queryId]?.inProgress = false
        activeQueries[queryId]?.timeoutTask?.cancel()
        lock.unlock()
    }
}

// MARK: - Network Utility

enum NetworkUtility {
    /// Returns the device's local WiFi IP address, or nil if unavailable.
    static func localIPAddress() -> String? {
        var address: String?
        var ifaddr: UnsafeMutablePointer<ifaddrs>?

        guard getifaddrs(&ifaddr) == 0, let firstAddr = ifaddr else {
            return nil
        }

        defer { freeifaddrs(ifaddr) }

        var ptr = firstAddr
        while true {
            let interface = ptr.pointee
            let addrFamily = interface.ifa_addr.pointee.sa_family

            if addrFamily == UInt8(AF_INET) {
                let name = String(cString: interface.ifa_name)
                // en0 is typically WiFi on iOS
                if name == "en0" {
                    var addr = interface.ifa_addr.pointee
                    var hostname = [CChar](repeating: 0, count: Int(NI_MAXHOST))
                    getnameinfo(
                        &addr,
                        socklen_t(interface.ifa_addr.pointee.sa_len),
                        &hostname,
                        socklen_t(hostname.count),
                        nil,
                        socklen_t(0),
                        NI_NUMERICHOST
                    )
                    address = String(cString: hostname)
                    break
                }
            }

            guard let next = interface.ifa_next else { break }
            ptr = next
        }

        return address
    }
}

// MARK: - Error

enum BroadcastError: Error, LocalizedError {
    case serializationFailed
    case sendFailed

    var errorDescription: String? {
        switch self {
        case .serializationFailed:
            return "Failed to serialize broadcast message"
        case .sendFailed:
            return "Failed to send broadcast"
        }
    }
}
