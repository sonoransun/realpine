import Foundation

struct Subnet: Codable, Sendable {
    let ipAddress: String
    let netMask: String
}
