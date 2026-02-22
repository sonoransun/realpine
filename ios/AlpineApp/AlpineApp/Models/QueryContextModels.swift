import Foundation

struct QueryRefinement: Codable, Sendable, Identifiable {
    let id: UUID
    let label: String
    let refinementType: RefinementType
    let parameters: RefinementParameters

    init(id: UUID = UUID(), label: String, refinementType: RefinementType, parameters: RefinementParameters) {
        self.id = id
        self.label = label
        self.refinementType = refinementType
        self.parameters = parameters
    }
}

enum RefinementType: String, Codable, Sendable {
    case addCategoryFilter
    case addLanguageFilter
    case addEntityFilter
    case addKeywordFilter
    case adjustThreshold
    case removeAmbiguity
}

struct RefinementParameters: Codable, Sendable {
    var contentCategory: ContentCategory?
    var languageCode: String?
    var entityType: EntityType?
    var entityText: String?
    var keyword: String?
    var threshold: Float?
}

struct SignalWeights: Codable, Sendable {
    var text: Double = 1.0
    var entity: Double = 1.0
    var category: Double = 0.8
    var language: Double = 0.5

    static let `default` = SignalWeights()
}
