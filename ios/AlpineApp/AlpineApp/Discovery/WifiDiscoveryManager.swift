import Foundation
import Network
import Observation
import os

private let logger = Logger(subsystem: "com.sonoranpub.AlpineApp", category: "WifiDiscoveryManager")

/// Listens on UDP port 8089 for Alpine REST Bridge beacon broadcasts.
/// Maintains a list of discovered bridges, automatically removing stale
/// entries that have not been seen within the threshold period.
@Observable
final class WifiDiscoveryManager {
    private(set) var discoveredBridges: [BridgeBeacon] = []
    private(set) var isScanning = false

    private var listener: NWListener?
    private var cleanupTask: Task<Void, Never>?
    private let staleThreshold: TimeInterval = 10.0
    private let cleanupInterval: TimeInterval = 3.0
    private let beaconPort: UInt16 = 8089
    private let queue = DispatchQueue(label: "com.alpine.discovery", qos: .utility)
    private let lock = NSLock()

    /// Starts listening for bridge beacon broadcasts on UDP port 8089.
    func start() {
        guard !isScanning else { return }

        do {
            let params = NWParameters.udp
            params.allowLocalEndpointReuse = true
            params.requiredInterfaceType = .wifi

            listener = try NWListener(using: params, on: NWEndpoint.Port(integerLiteral: beaconPort))
        } catch {
            logger.error("Failed to create listener: \(error.localizedDescription)")
            return
        }

        listener?.newConnectionHandler = { [weak self] connection in
            self?.handleNewConnection(connection)
        }

        listener?.stateUpdateHandler = { [weak self] state in
            switch state {
            case .ready:
                logger.info("Listening for beacons on port \(self?.beaconPort ?? 0)")
            case .failed(let error):
                logger.error("Listener failed: \(error.localizedDescription)")
                self?.stop()
            default:
                break
            }
        }

        listener?.start(queue: queue)
        isScanning = true
        startCleanup()
    }

    /// Stops listening for beacons and clears all discovered bridges.
    func stop() {
        listener?.cancel()
        listener = nil
        cleanupTask?.cancel()
        cleanupTask = nil

        lock.lock()
        let shouldClear = !discoveredBridges.isEmpty
        lock.unlock()

        if shouldClear {
            Task { @MainActor in
                self.discoveredBridges = []
            }
        }

        isScanning = false
    }

    // MARK: - Connection Handling

    private func handleNewConnection(_ connection: NWConnection) {
        connection.start(queue: queue)

        connection.receiveMessage { [weak self] data, _, _, error in
            defer { connection.cancel() }

            if let error {
                logger.error("Receive error: \(error.localizedDescription)")
                return
            }

            guard let data else { return }

            self?.handleBeacon(data: data, from: connection.currentPath?.remoteEndpoint)
        }
    }

    // MARK: - Beacon Processing

    private func handleBeacon(data: Data, from endpoint: NWEndpoint?) {
        let decoder = JSONDecoder()

        guard var beacon = try? decoder.decode(BridgeBeacon.self, from: data) else {
            return
        }

        // Extract the host address from the sender's endpoint
        if let endpoint {
            beacon.hostAddress = extractHost(from: endpoint)
        }

        // Skip beacons with no host address
        guard !beacon.hostAddress.isEmpty else { return }

        // Update or add the beacon on the main actor
        Task { @MainActor in
            self.updateBeacon(beacon)
        }
    }

    @MainActor
    private func updateBeacon(_ beacon: BridgeBeacon) {
        if let index = discoveredBridges.firstIndex(where: { $0.id == beacon.id }) {
            discoveredBridges[index] = beacon
        } else {
            discoveredBridges.append(beacon)
        }
    }

    /// Extracts the IP address string from a Network framework endpoint.
    private func extractHost(from endpoint: NWEndpoint) -> String {
        switch endpoint {
        case .hostPort(let host, _):
            switch host {
            case .ipv4(let addr):
                return "\(addr)"
            case .ipv6(let addr):
                return "\(addr)"
            case .name(let name, _):
                return name
            @unknown default:
                return ""
            }
        default:
            // Try string parsing as fallback
            let description = "\(endpoint)"
            // Endpoint description is typically "host:port"
            if let colonRange = description.range(of: ":", options: .backwards) {
                return String(description[description.startIndex..<colonRange.lowerBound])
            }
            return ""
        }
    }

    // MARK: - Stale Beacon Cleanup

    /// Periodically removes beacons that have not been refreshed within the stale threshold.
    private func startCleanup() {
        cleanupTask?.cancel()

        cleanupTask = Task { [weak self] in
            while !Task.isCancelled {
                try? await Task.sleep(for: .seconds(self?.cleanupInterval ?? 3.0))
                guard !Task.isCancelled else { break }
                guard let self else { break }

                await MainActor.run {
                    let now = Date()
                    self.discoveredBridges.removeAll { beacon in
                        now.timeIntervalSince(beacon.lastSeen) > self.staleThreshold
                    }
                }
            }
        }
    }
}
