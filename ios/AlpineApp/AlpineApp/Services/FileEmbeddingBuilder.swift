import Foundation

/// Builds sparse feature vectors (embeddings) from file metadata using TF-IDF
/// weighted keywords, entity annotations, file type, size buckets, and directory
/// components.
final class FileEmbeddingBuilder: Sendable {

    // MARK: - Public API

    /// Build embeddings for a collection of files using corpus-wide IDF weighting.
    func buildEmbeddings(for files: [FileMetadata]) -> [FileEmbedding] {
        let idf = computeIDF(files: files)
        return files.map { buildEmbedding(for: $0, idf: idf) }
    }

    /// Build a single file's embedding given precomputed IDF values.
    func buildEmbedding(for file: FileMetadata, idf: [String: Double]) -> FileEmbedding {
        var components: [String: Double] = [:]

        // Keywords weighted by IDF
        for keyword in file.keywords {
            let key = "kw:\(keyword)"
            let idfValue = idf[keyword, default: 1.0]
            components[key] = 2.0 * idfValue
        }

        // Entity annotations
        for entity in file.entities {
            let key = "ent:\(entity.entityType.rawValue):\(entity.text.lowercased())"
            components[key] = 3.0 * entity.confidence
        }

        // File type
        let typeKey = "type:\(file.fileExtension.lowercased())"
        components[typeKey] = 1.5

        // Size bucket
        let sizeKey = "size:\(sizeBucket(file.size))"
        components[sizeKey] = 0.5

        // Directory components
        for component in file.directoryComponents {
            let dirKey = "dir:\(component.lowercased())"
            components[dirKey] = 1.0
        }

        // Language
        if let lang = file.language {
            let langKey = "lang:\(lang.languageCode)"
            components[langKey] = 1.0 * lang.confidence
        }

        // Content category
        if let category = file.contentCategory {
            let catKey = "cat:\(category.rawValue)"
            components[catKey] = 1.5
        }

        return FileEmbedding(
            fileId: file.id,
            vector: FeatureVector(components: components),
            generatedAt: Date()
        )
    }

    /// Build a query vector from a search string and optional entity annotations.
    func buildQueryVector(
        queryString: String,
        entityAnnotations: [EntityAnnotation] = [],
        contentCategories: [ContentCategory] = [],
        languages: [String] = []
    ) -> FeatureVector {
        var components: [String: Double] = [:]

        // Tokenize query string
        let tokens = queryString
            .split(separator: " ")
            .map { $0.lowercased() }
            .filter { $0.count >= 2 }

        for token in tokens {
            let key = "kw:\(token)"
            components[key] = 2.0
        }

        // Entity annotations
        for entity in entityAnnotations {
            let key = "ent:\(entity.entityType.rawValue):\(entity.text.lowercased())"
            components[key] = 3.0 * entity.confidence
        }

        // Content category filters
        for category in contentCategories {
            let key = "cat:\(category.rawValue)"
            components[key] = 1.5
        }

        // Language filters
        for language in languages {
            let key = "lang:\(language)"
            components[key] = 1.0
        }

        return FeatureVector(components: components)
    }

    // MARK: - Private Helpers

    /// Compute Inverse Document Frequency for all keywords across the corpus.
    /// IDF = log(N / df) where df is the number of files containing the keyword.
    /// Minimum IDF is clamped to 0.1.
    private func computeIDF(files: [FileMetadata]) -> [String: Double] {
        let n = files.count
        guard n > 0 else { return [:] }

        // Count document frequency for each keyword
        var documentFrequency: [String: Int] = [:]
        for file in files {
            let uniqueKeywords = Set(file.keywords)
            for keyword in uniqueKeywords {
                documentFrequency[keyword, default: 0] += 1
            }
        }

        // Compute IDF
        let nDouble = Double(n)
        var idf: [String: Double] = [:]
        idf.reserveCapacity(documentFrequency.count)

        for (keyword, df) in documentFrequency {
            let value = log(nDouble / Double(df))
            idf[keyword] = max(value, 0.1)
        }

        return idf
    }

    /// Classify file size into a human-readable bucket label.
    private func sizeBucket(_ size: Int64) -> String {
        switch size {
        case ..<1_024:
            return "tiny"
        case ..<102_400:
            return "small"
        case ..<1_048_576:
            return "medium"
        case ..<104_857_600:
            return "large"
        default:
            return "huge"
        }
    }
}
