import Foundation

struct GroupInfo: Codable, Identifiable, Sendable {
    let groupId: Int64
    let groupName: String
    let description: String
    let numPeers: Int64
    let totalQueries: Int64
    let totalResponses: Int64

    var id: Int64 { groupId }
}
