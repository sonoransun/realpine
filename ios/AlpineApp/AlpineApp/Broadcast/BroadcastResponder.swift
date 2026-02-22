import Foundation
import Network

/// Listens on UDP port 8090 for incoming Alpine query broadcasts from peers
/// on the local network. When a query is received, searches local content
/// and sends back a response with matching file resources.
final class BroadcastResponder {
    private var listener: NWListener?
    private let port: UInt16 = 8090
    private let localContentProvider: LocalContentProvider
    private let fileServerPort: Int
    private let queue = DispatchQueue(label: "com.alpine.broadcast.responder", qos: .utility)

    /// Rate limiter: tracks query timestamps per senderId.
    private var rateLimiter: [String: [Date]] = [:]
    private let rateLock = NSLock()
    private let maxQueriesPerWindow = 5
    private let rateLimitWindow: TimeInterval = 10.0

    /// A unique responder ID for this device.
    private let responderId: String

    init(localContentProvider: LocalContentProvider, fileServerPort: Int) {
        self.localContentProvider = localContentProvider
        self.fileServerPort = fileServerPort
        self.responderId = String(UUID().uuidString.prefix(8)).lowercased()
    }

    /// Starts listening for query broadcasts on UDP port 8090.
    func start() {
        do {
            let params = NWParameters.udp
            params.allowLocalEndpointReuse = true
            params.requiredInterfaceType = .wifi

            listener = try NWListener(using: params, on: NWEndpoint.Port(integerLiteral: port))
        } catch {
            print("[BroadcastResponder] Failed to create listener: \(error)")
            return
        }

        listener?.newConnectionHandler = { [weak self] connection in
            self?.handleConnection(connection)
        }

        let listenPort = port
        listener?.stateUpdateHandler = { state in
            switch state {
            case .ready:
                print("[BroadcastResponder] Listening for queries on port \(listenPort)")
            case .failed(let error):
                print("[BroadcastResponder] Listener failed: \(error)")
            default:
                break
            }
        }

        listener?.start(queue: queue)
    }

    /// Stops the broadcast responder and cancels the listener.
    func stop() {
        listener?.cancel()
        listener = nil

        rateLock.lock()
        rateLimiter.removeAll()
        rateLock.unlock()
    }

    // MARK: - Connection Handling

    private func handleConnection(_ connection: NWConnection) {
        connection.start(queue: queue)

        connection.receiveMessage { [weak self] data, _, _, error in
            defer { connection.cancel() }

            if let error {
                print("[BroadcastResponder] Receive error: \(error)")
                return
            }

            guard let data else { return }

            self?.handleQuery(data: data, from: connection.currentPath?.remoteEndpoint)
        }
    }

    // MARK: - Query Handling

    private func handleQuery(data: Data, from endpoint: NWEndpoint?) {
        guard let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any] else {
            return
        }

        // Only process query messages
        guard let type = json["type"] as? String, type == "alpine_query" else {
            return
        }

        guard let queryId = json["queryId"] as? Int64,
              let senderId = json["senderId"] as? String,
              let queryString = json["queryString"] as? String else {
            return
        }

        // Do not respond to our own broadcasts
        if senderId == responderId {
            return
        }

        // Apply rate limiting per sender
        if isRateLimited(senderId: senderId) {
            print("[BroadcastResponder] Rate limited sender: \(senderId)")
            return
        }

        // Search local content
        let localAddress = NetworkUtility.localIPAddress() ?? "0.0.0.0"
        let results = localContentProvider.search(
            queryString: queryString,
            localAddress: localAddress,
            fileServerPort: fileServerPort
        )

        // Only send a response if we have matching results
        guard !results.isEmpty else { return }

        // Send the response back to the sender
        if let endpoint {
            sendResponse(queryId: queryId, resources: results, to: endpoint)
        } else if let senderAddress = json["senderAddress"] as? String,
                  let senderPort = json["senderPort"] as? Int {
            // Fall back to the address/port specified in the query message
            let target = NWEndpoint.hostPort(
                host: NWEndpoint.Host(senderAddress),
                port: NWEndpoint.Port(integerLiteral: UInt16(senderPort))
            )
            sendResponse(queryId: queryId, resources: results, to: target)
        }
    }

    // MARK: - Rate Limiting

    /// Checks whether the given senderId has exceeded the rate limit.
    /// Also records the current timestamp for the sender.
    private func isRateLimited(senderId: String) -> Bool {
        let now = Date()

        rateLock.lock()
        defer { rateLock.unlock() }

        // Clean up old entries outside the rate limit window
        var timestamps = rateLimiter[senderId] ?? []
        timestamps.removeAll { now.timeIntervalSince($0) > rateLimitWindow }

        if timestamps.count >= maxQueriesPerWindow {
            return true
        }

        timestamps.append(now)
        rateLimiter[senderId] = timestamps
        return false
    }

    // MARK: - Response Sending

    private func sendResponse(queryId: Int64, resources: [ResourceDesc], to endpoint: NWEndpoint) {
        // Build the resource array for JSON serialization
        let resourceDicts: [[String: Any]] = resources.map { resource in
            [
                "resourceId": resource.resourceId,
                "size": resource.size,
                "description": resource.description,
                "locators": resource.locators
            ]
        }

        let responseMessage: [String: Any] = [
            "type": "alpine_response",
            "queryId": queryId,
            "responderId": responderId,
            "resources": resourceDicts
        ]

        guard let data = try? JSONSerialization.data(withJSONObject: responseMessage) else {
            print("[BroadcastResponder] Failed to serialize response")
            return
        }

        let params = NWParameters.udp
        params.allowLocalEndpointReuse = true

        let connection = NWConnection(to: endpoint, using: params)
        connection.start(queue: queue)

        connection.send(content: data, completion: .contentProcessed { error in
            if let error {
                print("[BroadcastResponder] Send response error: \(error)")
            }
            // Brief delay before cancelling to ensure delivery
            DispatchQueue.global().asyncAfter(deadline: .now() + 0.3) {
                connection.cancel()
            }
        })
    }
}
