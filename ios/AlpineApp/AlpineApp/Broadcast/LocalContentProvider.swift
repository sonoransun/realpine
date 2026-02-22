import Foundation
import os

private let logger = Logger(subsystem: "com.sonoranpub.AlpineApp", category: "LocalContentProvider")

/// Indexes files in a shared directory and provides search capability
/// using glob-like patterns. Used by BroadcastResponder to answer
/// incoming query broadcasts with matching local files.
final class LocalContentProvider {
    private var indexedFiles: [(relativePath: String, size: Int64)] = []
    private var rootDirectory: String = ""
    private let maxIndexedFiles = 500
    private let maxQueryLength = 256
    private let maxResults = 500

    private var fileMetadata: [FileMetadata] = []
    private var fileEmbeddings: [FileEmbedding] = []
    private var knowledgeGraph = FileKnowledgeGraph()
    private let metadataExtractor = FileMetadataExtractor()
    private let embeddingBuilder = FileEmbeddingBuilder()

    var fileCount: Int { indexedFiles.count }
    var metadata: [FileMetadata] { fileMetadata }

    /// Recursively enumerates files in the given directory and builds an
    /// index of relative paths and file sizes, up to maxIndexedFiles.
    func indexDirectory(path: String) {
        logger.info("Indexing directory: \(path)")
        rootDirectory = path
        indexedFiles = []

        let fileManager = FileManager.default
        let rootURL = URL(fileURLWithPath: path, isDirectory: true)

        guard let enumerator = fileManager.enumerator(
            at: rootURL,
            includingPropertiesForKeys: [.fileSizeKey, .isRegularFileKey],
            options: [.skipsHiddenFiles, .skipsPackageDescendants]
        ) else {
            return
        }

        var count = 0
        while let fileURL = enumerator.nextObject() as? URL {
            guard count < maxIndexedFiles else { break }

            do {
                let resourceValues = try fileURL.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
                guard resourceValues.isRegularFile == true else { continue }

                let relativePath = fileURL.path.replacingOccurrences(
                    of: rootURL.path,
                    with: ""
                )
                // Strip leading slash from relative path
                let cleanPath = relativePath.hasPrefix("/")
                    ? String(relativePath.dropFirst())
                    : relativePath

                let size = Int64(resourceValues.fileSize ?? 0)
                indexedFiles.append((relativePath: cleanPath, size: size))
                count += 1
            } catch {
                continue
            }
        }

        // Build rich metadata for each indexed file
        fileMetadata = indexedFiles.map { file in
            metadataExtractor.extract(relativePath: file.relativePath, size: file.size, rootURL: rootURL)
        }

        // Populate knowledge graph
        knowledgeGraph.clear()
        for meta in fileMetadata {
            knowledgeGraph.addFileMetadata(meta)
        }

        // Build embeddings
        fileEmbeddings = embeddingBuilder.buildEmbeddings(for: fileMetadata)

        logger.info("Indexed \(self.indexedFiles.count) files with \(self.fileMetadata.count) metadata entries")
    }

    /// Searches indexed files using a glob-like pattern and returns matching
    /// ResourceDesc entries with HTTP locators pointing to the local file server.
    ///
    /// - Parameters:
    ///   - queryString: Glob pattern (supports * and ? wildcards)
    ///   - localAddress: The local IP address for building file URLs
    ///   - fileServerPort: The port the local file server is listening on
    /// - Returns: Array of ResourceDesc for matching files
    func search(queryString: String, localAddress: String, fileServerPort: Int) -> [ResourceDesc] {
        let trimmed = String(queryString.prefix(maxQueryLength))
            .trimmingCharacters(in: .whitespacesAndNewlines)

        guard !trimmed.isEmpty else { return [] }

        let regexPattern = globToRegex(trimmed)

        guard let regex = try? NSRegularExpression(
            pattern: regexPattern,
            options: .caseInsensitive
        ) else {
            return []
        }

        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        for file in indexedFiles {
            guard results.count < maxResults else { break }

            let range = NSRange(file.relativePath.startIndex..., in: file.relativePath)
            if regex.firstMatch(in: file.relativePath, range: range) != nil {
                let encodedPath = file.relativePath.addingPercentEncoding(
                    withAllowedCharacters: .urlPathAllowed
                ) ?? file.relativePath

                let locator = "http://\(localAddress):\(fileServerPort)/files/\(encodedPath)"

                let desc = ResourceDesc(
                    resourceId: resourceId,
                    size: file.size,
                    locators: [locator],
                    description: file.relativePath
                )
                results.append(desc)
                resourceId += 1
            }
        }

        return results
    }

    // MARK: - Enhanced Search

    /// Performs an enhanced search dispatching by the specified search mode.
    /// Supports keyword, semantic, entity, and SPARQL search strategies.
    func enhancedSearch(
        queryString: String,
        localAddress: String,
        fileServerPort: Int,
        searchMode: SearchMode,
        similarityMetric: SimilarityMetric = .cosine,
        similarityThreshold: Float = 0.3,
        entityFilters: [EntityFilter]? = nil,
        sparqlPatterns: [CodableTriplePattern]? = nil,
        contentCategories: [ContentCategory]? = nil,
        languages: [String]? = nil,
        signalWeights: SignalWeights? = nil
    ) -> [ResourceDesc] {
        switch searchMode {
        case .keyword:
            return keywordSearch(queryString: queryString, localAddress: localAddress, fileServerPort: fileServerPort)
        case .semantic:
            return semanticSearch(queryString: queryString, localAddress: localAddress, fileServerPort: fileServerPort, metric: similarityMetric, threshold: similarityThreshold)
        case .entity:
            return entitySearch(queryString: queryString, localAddress: localAddress, fileServerPort: fileServerPort, filters: entityFilters ?? [])
        case .sparql:
            return sparqlSearch(localAddress: localAddress, fileServerPort: fileServerPort, patterns: sparqlPatterns ?? [])
        case .multiModal:
            return multiModalSearch(
                queryString: queryString,
                localAddress: localAddress,
                fileServerPort: fileServerPort,
                contentCategories: contentCategories ?? [],
                languages: languages ?? [],
                weights: signalWeights ?? .default,
                metric: similarityMetric,
                threshold: similarityThreshold
            )
        }
    }

    // MARK: - Keyword Search

    /// Combines glob-based pattern matching with TF-IDF keyword scoring.
    /// Glob hits receive a score boost; keyword hits receive their cosine similarity score.
    /// Results are merged by taking the maximum score per file.
    private func keywordSearch(queryString: String, localAddress: String, fileServerPort: Int) -> [ResourceDesc] {
        let globResults = search(queryString: queryString, localAddress: localAddress, fileServerPort: fileServerPort)

        // Track scores by relative path: glob hits get a 0.3 boost
        var scoresByPath: [String: Double] = [:]
        var globPaths: Set<String> = []
        for result in globResults {
            scoresByPath[result.description] = 0.3
            globPaths.insert(result.description)
        }

        // TF-IDF keyword matching via embeddings
        let queryVector = embeddingBuilder.buildQueryVector(queryString: queryString, entityAnnotations: [])
        for (index, embedding) in fileEmbeddings.enumerated() {
            let cosineScore = SimilarityScoring.score(queryVector, embedding.vector, metric: .cosine)
            guard cosineScore > 0 else { continue }

            let path = fileMetadata[index].relativePath
            let existing = scoresByPath[path] ?? 0.0
            scoresByPath[path] = max(existing, cosineScore)
        }

        // Build results sorted by score descending
        let metaByPath = Dictionary(uniqueKeysWithValues: fileMetadata.map { ($0.relativePath, $0) })
        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        let sorted = scoresByPath.sorted { $0.value > $1.value }
        for (path, score) in sorted.prefix(maxResults) {
            guard let meta = metaByPath[path] else { continue }
            var desc = buildResourceDesc(from: meta, resourceId: resourceId, localAddress: localAddress, fileServerPort: fileServerPort)
            desc.score = Float(score)
            desc.matchedKeywords = meta.keywords
            resourceId += 1
            results.append(desc)
        }

        return results
    }

    // MARK: - Semantic Search

    /// Builds a query vector and ranks all file embeddings using the specified
    /// similarity metric and threshold.
    private func semanticSearch(queryString: String, localAddress: String, fileServerPort: Int, metric: SimilarityMetric, threshold: Float) -> [ResourceDesc] {
        let queryVector = embeddingBuilder.buildQueryVector(queryString: queryString, entityAnnotations: [])
        let candidateVectors = fileEmbeddings.map { $0.vector }
        let ranked = SimilarityScoring.rank(query: queryVector, candidates: candidateVectors, metric: metric, threshold: Double(threshold), limit: maxResults)

        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        for entry in ranked {
            let meta = fileMetadata[entry.index]
            var desc = buildResourceDesc(from: meta, resourceId: resourceId, localAddress: localAddress, fileServerPort: fileServerPort)
            desc.score = Float(entry.score)
            desc.matchedEntities = meta.entities
            desc.matchedKeywords = meta.keywords
            resourceId += 1
            results.append(desc)
        }

        return results
    }

    // MARK: - Entity Search

    /// Searches file metadata for entities matching the provided filters.
    /// Scores each file by the fraction of filters matched. If queryString is
    /// non-empty, also checks for case-insensitive entity text containment.
    private func entitySearch(queryString: String, localAddress: String, fileServerPort: Int, filters: [EntityFilter]) -> [ResourceDesc] {
        guard !filters.isEmpty else { return [] }

        let queryLower = queryString.lowercased()
        var scoredResults: [(meta: FileMetadata, score: Double, matched: [EntityAnnotation])] = []

        for meta in fileMetadata {
            var matchedEntities: [EntityAnnotation] = []

            for filter in filters {
                let filterTextLower = filter.text.lowercased()
                for entity in meta.entities {
                    let textMatch = entity.text.lowercased().contains(filterTextLower)
                    let typeMatch = filter.entityType == nil || entity.entityType == filter.entityType
                    let confidenceMatch = entity.confidence >= filter.minConfidence

                    if textMatch && typeMatch && confidenceMatch {
                        matchedEntities.append(entity)
                    }
                }
            }

            // Also check if entity text contains the query string
            if !queryLower.isEmpty {
                for entity in meta.entities {
                    if entity.text.lowercased().contains(queryLower) && !matchedEntities.contains(where: { $0.id == entity.id }) {
                        matchedEntities.append(entity)
                    }
                }
            }

            guard !matchedEntities.isEmpty else { continue }

            let score = Double(matchedEntities.count) / Double(filters.count)
            scoredResults.append((meta: meta, score: score, matched: matchedEntities))
        }

        // Sort by score descending
        scoredResults.sort { $0.score > $1.score }

        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        for entry in scoredResults.prefix(maxResults) {
            var desc = buildResourceDesc(from: entry.meta, resourceId: resourceId, localAddress: localAddress, fileServerPort: fileServerPort)
            desc.score = Float(entry.score)
            desc.matchedEntities = entry.matched
            resourceId += 1
            results.append(desc)
        }

        return results
    }

    // MARK: - SPARQL Search

    /// Converts codable triple patterns to RDF triple patterns and executes
    /// a basic graph pattern (BGP) query against the knowledge graph.
    private func sparqlSearch(localAddress: String, fileServerPort: Int, patterns: [CodableTriplePattern]) -> [ResourceDesc] {
        guard !patterns.isEmpty else { return [] }

        // Convert CodableTriplePattern to TriplePattern
        let triplePatterns: [TriplePattern] = patterns.map { codable in
            let subject: RDFNode? = codable.subject.map { str in
                str.hasPrefix("file:") ? .uri(str) : .literal(str, .string)
            }
            let object: RDFNode? = codable.object.map { str in
                str.hasPrefix("file:") ? .uri(str) : .literal(str, .string)
            }
            return TriplePattern(subject: subject, predicate: codable.predicate, object: object)
        }

        let matchingNodes = knowledgeGraph.queryBGP(patterns: triplePatterns)

        // Build a lookup from URI to FileMetadata (URIs match addFileMetadata format)
        let metaByURI = Dictionary(uniqueKeysWithValues: fileMetadata.map { ("file:\($0.id)", $0) })

        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        for node in matchingNodes {
            guard case .uri(let uri) = node, let meta = metaByURI[uri] else { continue }

            // Gather matching triples for this file
            let fileTriples = triplePatterns.flatMap { pattern in
                let boundPattern = TriplePattern(subject: node, predicate: pattern.predicate, object: pattern.object)
                return knowledgeGraph.query(pattern: boundPattern)
            }

            var desc = buildResourceDesc(from: meta, resourceId: resourceId, localAddress: localAddress, fileServerPort: fileServerPort)
            desc.rdfTriples = fileTriples
            resourceId += 1
            results.append(desc)
        }

        return results
    }

    // MARK: - Multi-Modal Search

    /// Combines weighted text similarity, entity matching, category filtering,
    /// and language signals into a unified search.
    private func multiModalSearch(
        queryString: String,
        localAddress: String,
        fileServerPort: Int,
        contentCategories: [ContentCategory],
        languages: [String],
        weights: SignalWeights,
        metric: SimilarityMetric,
        threshold: Float
    ) -> [ResourceDesc] {
        let queryVector = embeddingBuilder.buildQueryVector(
            queryString: queryString,
            entityAnnotations: [],
            contentCategories: contentCategories,
            languages: languages
        )

        var scoredResults: [(meta: FileMetadata, score: Double)] = []

        for (index, embedding) in fileEmbeddings.enumerated() {
            let meta = fileMetadata[index]

            // Text signal from vector similarity
            let textScore = SimilarityScoring.score(queryVector, embedding.vector, metric: metric)

            // Entity match bonus: 0.2 per matching entity mention
            let queryLower = queryString.lowercased()
            var entityScore = 0.0
            if !queryLower.isEmpty {
                for entity in meta.entities {
                    if entity.text.lowercased().contains(queryLower) || queryLower.contains(entity.text.lowercased()) {
                        entityScore += 0.2
                    }
                }
            }
            entityScore = min(entityScore, 1.0)

            // Category match: 1.0 if file matches requested categories
            var categoryScore = 0.0
            if !contentCategories.isEmpty, let fileCat = meta.contentCategory {
                categoryScore = contentCategories.contains(fileCat) ? 1.0 : 0.0
            }

            // Language match: 1.0 if file matches requested languages
            var languageScore = 0.0
            if !languages.isEmpty, let fileLang = meta.language {
                languageScore = languages.contains(fileLang.languageCode) ? 1.0 : 0.0
            }

            // Compute weighted composite score
            var totalWeight = weights.text
            var composite = textScore * weights.text

            if entityScore > 0 || !queryLower.isEmpty {
                totalWeight += weights.entity
                composite += entityScore * weights.entity
            }
            if !contentCategories.isEmpty {
                totalWeight += weights.category
                composite += categoryScore * weights.category
            }
            if !languages.isEmpty {
                totalWeight += weights.language
                composite += languageScore * weights.language
            }

            let finalScore = totalWeight > 0 ? composite / totalWeight : 0.0

            guard finalScore >= Double(threshold) else { continue }
            scoredResults.append((meta: meta, score: finalScore))
        }

        // Sort by composite score descending
        scoredResults.sort { $0.score > $1.score }

        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        for entry in scoredResults.prefix(maxResults) {
            var desc = buildResourceDesc(from: entry.meta, resourceId: resourceId, localAddress: localAddress, fileServerPort: fileServerPort)
            desc.score = Float(entry.score)
            desc.matchedEntities = entry.meta.entities
            desc.matchedKeywords = entry.meta.keywords
            desc.contentCategory = entry.meta.contentCategory
            desc.language = entry.meta.language
            desc.mediaMetadata = entry.meta.mediaMetadata
            resourceId += 1
            results.append(desc)
        }

        // Generate refinement suggestions and attach to first result
        if !results.isEmpty {
            let refinements = generateRefinements(results: results, query: queryString)
            if !refinements.isEmpty {
                results[0].refinements = refinements
            }
        }

        return results
    }

    // MARK: - Refinement Suggestions

    /// Analyzes the result set to produce up to 5 QueryRefinement suggestions.
    private func generateRefinements(results: [ResourceDesc], query: String) -> [QueryRefinement] {
        var refinements: [QueryRefinement] = []

        // 1. Category narrowing: if results span multiple categories, suggest most common
        var categoryCounts: [ContentCategory: Int] = [:]
        for result in results {
            if let cat = result.contentCategory {
                categoryCounts[cat, default: 0] += 1
            }
        }
        if categoryCounts.count > 1, let (topCat, topCount) = categoryCounts.max(by: { $0.value < $1.value }) {
            refinements.append(QueryRefinement(
                label: "Narrow to \(topCat.rawValue)s (\(topCount) results)",
                refinementType: .addCategoryFilter,
                parameters: RefinementParameters(contentCategory: topCat)
            ))
        }

        // 2. Language narrowing: if results contain multiple languages, suggest dominant
        var langCounts: [String: Int] = [:]
        for result in results {
            if let lang = result.language {
                langCounts[lang.languageCode, default: 0] += 1
            }
        }
        if langCounts.count > 1, let (topLang, topCount) = langCounts.max(by: { $0.value < $1.value }) {
            let displayName = Locale.current.localizedString(forLanguageCode: topLang) ?? topLang
            refinements.append(QueryRefinement(
                label: "Filter to \(displayName) (\(topCount) results)",
                refinementType: .addLanguageFilter,
                parameters: RefinementParameters(languageCode: topLang)
            ))
        }

        // 3. Entity narrowing: top 2 most frequent entity types
        var entityTypeCounts: [EntityType: Int] = [:]
        for result in results {
            if let entities = result.matchedEntities {
                for entity in entities {
                    entityTypeCounts[entity.entityType, default: 0] += 1
                }
            }
        }
        let topEntityTypes = entityTypeCounts.sorted { $0.value > $1.value }.prefix(2)
        for (entityType, count) in topEntityTypes {
            guard refinements.count < 5 else { break }
            refinements.append(QueryRefinement(
                label: "Filter by \(entityType.rawValue) (\(count) matches)",
                refinementType: .addEntityFilter,
                parameters: RefinementParameters(entityType: entityType)
            ))
        }

        // 4. Threshold adjustment: if many results score below 0.5
        let lowScoreCount = results.filter { ($0.score ?? 0) < 0.5 }.count
        if lowScoreCount > results.count / 2 && refinements.count < 5 {
            refinements.append(QueryRefinement(
                label: "Raise threshold to 50%",
                refinementType: .adjustThreshold,
                parameters: RefinementParameters(threshold: 0.5)
            ))
        }

        // 5. Ambiguity detection: high result count with broad distribution
        if results.count > 50 && refinements.count < 5 {
            refinements.append(QueryRefinement(
                label: "Add keywords to narrow results",
                refinementType: .removeAmbiguity,
                parameters: RefinementParameters(keyword: query)
            ))
        }

        return Array(refinements.prefix(5))
    }

    // MARK: - Helpers

    /// Builds a ResourceDesc from FileMetadata with the standard HTTP locator URL.
    private func buildResourceDesc(from meta: FileMetadata, resourceId: Int64, localAddress: String, fileServerPort: Int) -> ResourceDesc {
        let encodedPath = meta.relativePath.addingPercentEncoding(withAllowedCharacters: .urlPathAllowed) ?? meta.relativePath
        let locator = "http://\(localAddress):\(fileServerPort)/files/\(encodedPath)"
        return ResourceDesc(
            resourceId: resourceId,
            size: meta.size,
            locators: [locator],
            description: meta.relativePath
        )
    }

    /// Converts a glob-like pattern to a regular expression.
    /// Supports * (match any sequence) and ? (match single character).
    private func globToRegex(_ glob: String) -> String {
        var regex = "^"
        for char in glob {
            switch char {
            case "*":
                regex += ".*"
            case "?":
                regex += "."
            case ".":
                regex += "\\."
            case "(", ")", "[", "]", "{", "}", "+", "^", "$", "|", "\\":
                regex += "\\\(char)"
            default:
                regex += String(char)
            }
        }
        regex += "$"
        return regex
    }
}
