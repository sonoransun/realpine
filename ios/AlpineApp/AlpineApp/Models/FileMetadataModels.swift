import Foundation

enum EntityType: String, Codable, Sendable, CaseIterable {
    case personalName, placeName, organizationName, date, fileType
}

enum EntitySource: String, Codable, Sendable {
    case filename, directoryPath, fileExtension
}

struct EntityAnnotation: Codable, Sendable, Identifiable, Hashable {
    let id: UUID
    let text: String
    let entityType: EntityType
    let confidence: Double
    let source: EntitySource

    init(id: UUID = UUID(), text: String, entityType: EntityType, confidence: Double, source: EntitySource) {
        self.id = id
        self.text = text
        self.entityType = entityType
        self.confidence = confidence
        self.source = source
    }
}

struct FileMetadata: Codable, Sendable, Identifiable {
    let id: UUID
    let relativePath: String
    let filename: String
    let fileExtension: String
    let size: Int64
    let keywords: [String]
    let entities: [EntityAnnotation]
    let directoryComponents: [String]
    var contentCategory: ContentCategory?
    var language: LanguageInfo?
    var mediaMetadata: MediaMetadata?

    init(
        id: UUID = UUID(),
        relativePath: String,
        filename: String,
        fileExtension: String,
        size: Int64,
        keywords: [String],
        entities: [EntityAnnotation],
        directoryComponents: [String],
        contentCategory: ContentCategory? = nil,
        language: LanguageInfo? = nil,
        mediaMetadata: MediaMetadata? = nil
    ) {
        self.id = id
        self.relativePath = relativePath
        self.filename = filename
        self.fileExtension = fileExtension
        self.size = size
        self.keywords = keywords
        self.entities = entities
        self.directoryComponents = directoryComponents
        self.contentCategory = contentCategory
        self.language = language
        self.mediaMetadata = mediaMetadata
    }
}

struct FileEmbedding: Sendable {
    let fileId: UUID
    let vector: FeatureVector
    let generatedAt: Date
}
