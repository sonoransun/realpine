import Foundation
import NaturalLanguage

/// Extracts structured metadata from file paths using tokenization, NLP entity
/// recognition, and pattern-based detection.
final class FileMetadataExtractor: Sendable {

    private let languageDetector = LanguageDetector()
    private let contentClassifier = ContentClassifier()
    private let mediaInspector = MediaInspector()

    // MARK: - Public API

    /// Extract full metadata from a relative file path and file size.
    func extract(relativePath: String, size: Int64, rootURL: URL? = nil) -> FileMetadata {
        let url = URL(fileURLWithPath: relativePath)
        let filename = url.lastPathComponent
        let fileExtension = url.pathExtension.lowercased()
        let directoryComponents = extractDirectoryComponents(relativePath)
        let tokens = tokenizePath(relativePath)
        let keywords = normalizeTokens(tokens)
        let entities = detectEntities(
            relativePath: relativePath,
            tokens: tokens
        )

        // Detect language from tokens
        let language = languageDetector.detectLanguage(tokens: tokens)

        // Classify content category
        let contentCategory = contentClassifier.classify(fileExtension: fileExtension)

        // Extract media metadata if rootURL provided
        var absoluteURL: URL?
        if let rootURL {
            absoluteURL = rootURL.appendingPathComponent(relativePath)
        }
        let mediaMetadata = mediaInspector.inspect(fileURL: absoluteURL, contentCategory: contentCategory)

        return FileMetadata(
            relativePath: relativePath,
            filename: filename,
            fileExtension: fileExtension,
            size: size,
            keywords: keywords,
            entities: entities,
            directoryComponents: directoryComponents,
            contentCategory: contentCategory,
            language: language,
            mediaMetadata: mediaMetadata
        )
    }

    // MARK: - Tokenization

    /// Splits a path into tokens by decomposing each component on camelCase
    /// boundaries, underscores, hyphens, and dots. The file extension is stripped
    /// from the last component before tokenizing.
    func tokenizePath(_ path: String) -> [String] {
        let components = path.split(separator: "/").map(String.init)
        guard !components.isEmpty else { return [] }

        var tokens: [String] = []

        for (index, component) in components.enumerated() {
            var text = component
            // Strip extension from the last component (the filename)
            if index == components.count - 1 {
                if let dotRange = text.range(of: ".", options: .backwards) {
                    text = String(text[text.startIndex..<dotRange.lowerBound])
                }
            }
            tokens.append(contentsOf: splitComponent(text))
        }

        return tokens
    }

    /// Extract all path components except the last (filename).
    func extractDirectoryComponents(_ path: String) -> [String] {
        let components = path.split(separator: "/").map(String.init)
        guard components.count > 1 else { return [] }
        return Array(components.dropLast())
    }

    /// Lowercase tokens, remove those shorter than 2 characters, and deduplicate
    /// while preserving first-occurrence order.
    func normalizeTokens(_ tokens: [String]) -> [String] {
        var seen = Set<String>()
        var result: [String] = []

        for token in tokens {
            let lowered = token.lowercased()
            guard lowered.count >= 2, !seen.contains(lowered) else { continue }
            seen.insert(lowered)
            result.append(lowered)
        }

        return result
    }

    // MARK: - Entity Detection

    /// Detect named entities using NLTagger and pattern-based heuristics.
    func detectEntities(relativePath: String, tokens: [String]) -> [EntityAnnotation] {
        var annotations: [EntityAnnotation] = []

        // 1. NLP-based entity recognition on joined tokens
        let text = tokens.joined(separator: " ")
        annotations.append(contentsOf: nlpEntities(from: text))

        // 2. Pattern-based date detection in tokens
        annotations.append(contentsOf: dateEntities(from: tokens))

        // 3. File type entity from the extension
        let url = URL(fileURLWithPath: relativePath)
        let ext = url.pathExtension.lowercased()
        if !ext.isEmpty {
            let fileTypeLabel = fileTypeLabel(for: ext)
            annotations.append(EntityAnnotation(
                text: fileTypeLabel,
                entityType: .fileType,
                confidence: 1.0,
                source: .fileExtension
            ))
        }

        return annotations
    }

    // MARK: - Private Helpers

    /// Split a single path component on camelCase boundaries, underscores,
    /// hyphens, and dots.
    private func splitComponent(_ text: String) -> [String] {
        // First split on underscores, hyphens, and dots
        let delimitedParts = text
            .replacingOccurrences(of: "_", with: " ")
            .replacingOccurrences(of: "-", with: " ")
            .replacingOccurrences(of: ".", with: " ")
            .split(separator: " ")
            .map(String.init)

        var result: [String] = []
        for part in delimitedParts {
            result.append(contentsOf: splitCamelCase(part))
        }
        return result
    }

    /// Split a string on camelCase boundaries.
    /// "MonthlyReport" -> ["Monthly", "Report"]
    /// "XMLParser" -> ["XML", "Parser"]
    /// "report2024" -> ["report", "2024"]
    private func splitCamelCase(_ text: String) -> [String] {
        guard !text.isEmpty else { return [] }

        var tokens: [String] = []
        var current = ""
        let chars = Array(text)

        for i in 0..<chars.count {
            let ch = chars[i]

            if i == 0 {
                current.append(ch)
                continue
            }

            let prev = chars[i - 1]
            let isCurrentUpper = ch.isUppercase
            let isPrevUpper = prev.isUppercase
            let isCurrentDigit = ch.isNumber
            let isPrevDigit = prev.isNumber

            // Transition: lowercase/digit -> uppercase
            if isCurrentUpper && !isPrevUpper {
                if !current.isEmpty {
                    tokens.append(current)
                }
                current = String(ch)
            }
            // Transition: uppercase -> uppercase followed by lowercase (e.g., "XMLParser")
            else if isCurrentUpper && isPrevUpper {
                let nextIsLower = (i + 1 < chars.count) && chars[i + 1].isLowercase
                if nextIsLower && current.count > 1 {
                    tokens.append(current)
                    current = String(ch)
                } else {
                    current.append(ch)
                }
            }
            // Transition: letter <-> digit boundary
            else if isCurrentDigit != isPrevDigit && !isPrevUpper && !isCurrentUpper {
                if !current.isEmpty {
                    tokens.append(current)
                }
                current = String(ch)
            } else if isCurrentDigit != isPrevDigit && (isPrevUpper || isCurrentUpper) {
                if !current.isEmpty {
                    tokens.append(current)
                }
                current = String(ch)
            } else {
                current.append(ch)
            }
        }

        if !current.isEmpty {
            tokens.append(current)
        }

        return tokens
    }

    /// Use NLTagger to detect personal names, place names, and organizations.
    private func nlpEntities(from text: String) -> [EntityAnnotation] {
        let tagger = NLTagger(tagSchemes: [.nameType])
        tagger.string = text

        var annotations: [EntityAnnotation] = []
        let options: NLTagger.Options = [.omitWhitespace, .omitPunctuation]

        tagger.enumerateTags(
            in: text.startIndex..<text.endIndex,
            unit: .word,
            scheme: .nameType,
            options: options
        ) { tag, tokenRange in
            guard let tag else { return true }

            let entityType: EntityType?
            switch tag {
            case .personalName:
                entityType = .personalName
            case .placeName:
                entityType = .placeName
            case .organizationName:
                entityType = .organizationName
            default:
                entityType = nil
            }

            if let entityType {
                let word = String(text[tokenRange])
                // NER confidence heuristic: longer matches are more reliable
                let confidence = word.count >= 4 ? 0.9 : 0.7
                annotations.append(EntityAnnotation(
                    text: word,
                    entityType: entityType,
                    confidence: confidence,
                    source: .filename
                ))
            }

            return true
        }

        return annotations
    }

    /// Pattern-based date detection using regex for common formats.
    private func dateEntities(from tokens: [String]) -> [EntityAnnotation] {
        let patterns = [
            #"\b\d{4}-\d{2}-\d{2}\b"#,        // YYYY-MM-DD
            #"\b\d{2}-\d{2}-\d{4}\b"#,         // MM-DD-YYYY
            #"\b\d{4}\d{2}\d{2}\b"#,            // YYYYMMDD
            #"\b(19|20)\d{2}\b"#                 // YYYY (1900-2099)
        ]

        var annotations: [EntityAnnotation] = []
        let fullText = tokens.joined(separator: " ")

        for pattern in patterns {
            guard let regex = try? NSRegularExpression(pattern: pattern) else { continue }
            let range = NSRange(fullText.startIndex..., in: fullText)
            let matches = regex.matches(in: fullText, range: range)

            for match in matches {
                guard let swiftRange = Range(match.range, in: fullText) else { continue }
                let dateText = String(fullText[swiftRange])
                // Avoid duplicate detection of the same text
                if annotations.contains(where: { $0.text == dateText && $0.entityType == .date }) {
                    continue
                }
                annotations.append(EntityAnnotation(
                    text: dateText,
                    entityType: .date,
                    confidence: 0.95,
                    source: .filename
                ))
            }
        }

        return annotations
    }

    /// Map a file extension to a human-readable file type label.
    private func fileTypeLabel(for ext: String) -> String {
        switch ext {
        case "pdf": return "PDF Document"
        case "doc", "docx": return "Word Document"
        case "xls", "xlsx": return "Excel Spreadsheet"
        case "ppt", "pptx": return "PowerPoint Presentation"
        case "jpg", "jpeg": return "JPEG Image"
        case "png": return "PNG Image"
        case "gif": return "GIF Image"
        case "svg": return "SVG Image"
        case "mp3": return "MP3 Audio"
        case "mp4": return "MP4 Video"
        case "mov": return "QuickTime Video"
        case "zip": return "ZIP Archive"
        case "tar", "gz", "tgz": return "Compressed Archive"
        case "txt": return "Text File"
        case "csv": return "CSV Data"
        case "json": return "JSON Data"
        case "xml": return "XML Data"
        case "html", "htm": return "HTML Document"
        case "swift": return "Swift Source"
        case "cpp", "cc", "cxx": return "C++ Source"
        case "h", "hpp": return "Header File"
        case "py": return "Python Source"
        case "js": return "JavaScript Source"
        case "ts": return "TypeScript Source"
        case "md": return "Markdown Document"
        default: return "\(ext.uppercased()) File"
        }
    }
}
