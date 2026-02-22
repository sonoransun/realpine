import Foundation

struct QueryRequest: Codable, Sendable {
    let queryString: String
    var groupName: String = ""
    var autoHaltLimit: Int64 = 100
    var peerDescMax: Int64 = 50
}

struct QueryResponse: Codable, Sendable {
    let queryId: Int64
}

struct QueryStatusResponse: Codable, Sendable {
    let inProgress: Bool
    let totalPeers: Int64
    let peersQueried: Int64
    let numPeerResponses: Int64
    let totalHits: Int64
}

struct QueryResultsResponse: Codable, Sendable {
    let peers: [PeerResources]
}

struct PeerResources: Codable, Identifiable, Sendable {
    let peerId: Int64
    let resources: [ResourceDesc]

    var id: Int64 { peerId }
}

struct ResourceDesc: Codable, Identifiable, Sendable {
    let resourceId: Int64
    let size: Int64
    let locators: [String]
    let description: String

    var id: Int64 { resourceId }
}
