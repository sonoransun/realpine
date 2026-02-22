import Foundation

struct PeerSummary: Codable, Identifiable, Sendable {
    let peerId: Int64
    let ipAddress: String
    let port: Int
    let active: Bool
    let avgBandwidth: Int64
    let peakBandwidth: Int64

    var id: Int64 { peerId }
}

struct PeerDetail: Codable, Identifiable, Sendable {
    let peerId: Int64
    let ipAddress: String
    let port: Int64
    let lastRecvTime: Int64
    let lastSendTime: Int64
    let avgBandwidth: Int64
    let peakBandwidth: Int64

    var id: Int64 { peerId }
}

struct PeerListResponse: Codable, Sendable {
    let peers: [PeerSummary]
}
