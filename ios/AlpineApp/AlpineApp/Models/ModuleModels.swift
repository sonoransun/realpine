import Foundation

struct ModuleInfo: Codable, Identifiable, Sendable {
    let moduleId: Int64
    let moduleName: String
    let description: String
    let version: String
    let libraryPath: String
    let bootstrapSymbol: String
    let activeTime: Int64

    var id: Int64 { moduleId }
}
