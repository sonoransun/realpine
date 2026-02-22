import Foundation

enum TransportMode: String, Codable, CaseIterable, Sendable {
    case restBridge = "REST_BRIDGE"
    case wifiBroadcast = "WIFI_BROADCAST"
}
