import Foundation

// MARK: - Search Configuration Types

enum SearchMode: String, Codable, Sendable, CaseIterable {
    case keyword, semantic, entity, sparql, multiModal
}

struct EntityFilter: Codable, Sendable {
    let text: String
    let entityType: EntityType?
    let minConfidence: Double
}

struct CodableTriplePattern: Codable, Sendable {
    let subject: String?
    let predicate: String?
    let object: String?
}

enum ResultSortOrder: String, CaseIterable, Sendable {
    case relevance, name, size
}

// MARK: - Query Request / Response

struct QueryRequest: Codable, Sendable {
    let queryString: String
    var groupName: String = ""
    var autoHaltLimit: Int64 = 100
    var peerDescMax: Int64 = 50
    var searchMode: SearchMode?
    var similarityMetric: SimilarityMetric?
    var similarityThreshold: Float?
    var entityFilters: [EntityFilter]?
    var sparqlPatterns: [CodableTriplePattern]?
    var protocolVersion: Int?
    var contentCategories: [ContentCategory]?
    var languages: [String]?
    var signalWeights: SignalWeights?
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
    var score: Float?
    var matchedEntities: [EntityAnnotation]?
    var matchedKeywords: [String]?
    var rdfTriples: [RDFTriple]?
    var contentCategory: ContentCategory?
    var language: LanguageInfo?
    var mediaMetadata: MediaMetadata?
    var refinements: [QueryRefinement]?

    var id: Int64 { resourceId }
}
