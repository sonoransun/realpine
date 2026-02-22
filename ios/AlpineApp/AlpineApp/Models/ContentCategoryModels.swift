import Foundation

enum ContentCategory: String, Codable, Sendable, CaseIterable {
    case document, image, audio, video, archive, code, data, other
}

struct MediaMetadata: Codable, Sendable, Hashable {
    var width: Int?
    var height: Int?
    var durationSeconds: Double?
    var colorSpace: String?
    var hasAlpha: Bool?
    var codec: String?
    var bitRate: Int?
    var sampleRate: Int?
}

struct LanguageInfo: Codable, Sendable, Hashable {
    let languageCode: String
    let confidence: Double
}
