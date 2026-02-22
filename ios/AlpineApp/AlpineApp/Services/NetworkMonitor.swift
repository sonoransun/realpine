import Network
import Observation

/// Monitors network connectivity and reports path status changes.
@Observable
final class NetworkMonitor {
    enum ConnectionType: String {
        case wifi
        case cellular
        case ethernet
        case unknown
    }

    private(set) var isConnected: Bool = true
    private(set) var connectionType: ConnectionType = .unknown
    private(set) var isExpensive: Bool = false
    private(set) var isConstrained: Bool = false

    private let monitor: NWPathMonitor
    private let queue = DispatchQueue(label: "com.sonoranpub.AlpineApp.NetworkMonitor")

    init() {
        monitor = NWPathMonitor()
        start()
    }

    deinit {
        stop()
    }

    func start() {
        monitor.pathUpdateHandler = { [weak self] path in
            guard let self else { return }
            let connected = path.status == .satisfied
            let expensive = path.isExpensive
            let constrained = path.isConstrained
            let type = self.resolveConnectionType(from: path)
            DispatchQueue.main.async {
                self.isConnected = connected
                self.isExpensive = expensive
                self.isConstrained = constrained
                self.connectionType = type
            }
        }
        monitor.start(queue: queue)
    }

    func stop() {
        monitor.cancel()
    }

    private func resolveConnectionType(from path: NWPath) -> ConnectionType {
        if path.usesInterfaceType(.wifi) {
            return .wifi
        } else if path.usesInterfaceType(.cellular) {
            return .cellular
        } else if path.usesInterfaceType(.wiredEthernet) {
            return .ethernet
        } else {
            return .unknown
        }
    }
}
