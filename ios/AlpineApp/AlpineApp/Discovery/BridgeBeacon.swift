import Foundation

struct BridgeBeacon: Codable, Identifiable, Sendable {
    let service: String
    let protocolVersion: String
    let restPort: Int
    let bridgeVersion: String
    var hostAddress: String = ""
    var lastSeen: Date = Date()

    var id: String { "\(hostAddress):\(restPort)" }

    enum CodingKeys: String, CodingKey {
        case service
        case protocolVersion = "version"
        case restPort
        case bridgeVersion
        case hostAddress
    }

    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        service = try container.decode(String.self, forKey: .service)
        protocolVersion = try container.decode(String.self, forKey: .protocolVersion)
        restPort = try container.decode(Int.self, forKey: .restPort)
        bridgeVersion = try container.decode(String.self, forKey: .bridgeVersion)
        hostAddress = try container.decodeIfPresent(String.self, forKey: .hostAddress) ?? ""
        lastSeen = Date()
    }

    init(service: String, protocolVersion: String, restPort: Int, bridgeVersion: String, hostAddress: String) {
        self.service = service
        self.protocolVersion = protocolVersion
        self.restPort = restPort
        self.bridgeVersion = bridgeVersion
        self.hostAddress = hostAddress
        self.lastSeen = Date()
    }
}
