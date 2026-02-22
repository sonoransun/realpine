import Foundation
import UniformTypeIdentifiers

final class ContentClassifier: Sendable {

    /// Classify file into a content category based on its extension.
    func classify(fileExtension: String) -> ContentCategory {
        guard !fileExtension.isEmpty,
              let utType = UTType(filenameExtension: fileExtension) else {
            return .other
        }

        if utType.conforms(to: .image) { return .image }
        if utType.conforms(to: .audio) { return .audio }
        if utType.conforms(to: .movie) || utType.conforms(to: .video) { return .video }
        if utType.conforms(to: .archive) || utType.conforms(to: .zip) { return .archive }
        if utType.conforms(to: .sourceCode) { return .code }
        if utType.conforms(to: .json) || utType.conforms(to: .xml) || utType.conforms(to: .spreadsheet) || utType.conforms(to: .database) { return .data }
        if utType.conforms(to: .text) || utType.conforms(to: .pdf) || utType.conforms(to: .presentation) { return .document }

        return .other
    }
}
