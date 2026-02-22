import Foundation

struct ServerStatus: Codable, Sendable {
    let status: String
    let version: String
}
