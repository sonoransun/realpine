import Testing
@testable import AlpineApp

@Suite("Vector Similarity Tests")
struct VectorSimilarityTests {

    // MARK: - FeatureVector Tests

    @Test("FeatureVector magnitude")
    func featureVectorMagnitude() {
        let v = FeatureVector(components: ["a": 3.0, "b": 4.0])
        #expect(abs(v.magnitude - 5.0) < 0.001)
    }

    @Test("FeatureVector empty magnitude is zero")
    func emptyVectorMagnitude() {
        let v = FeatureVector(components: [:])
        #expect(v.magnitude == 0.0)
    }

    @Test("FeatureVector normalization sums to 1")
    func featureVectorNormalization() {
        let v = FeatureVector(components: ["a": 2.0, "b": 3.0, "c": 5.0])
        let normalized = v.normalized()
        let sum = normalized.components.values.reduce(0.0, +)
        #expect(abs(sum - 1.0) < 0.001)
    }

    @Test("FeatureVector subscript access")
    func featureVectorSubscript() {
        let v = FeatureVector(components: ["x": 1.5])
        #expect(v["x"] == 1.5)
        #expect(v["y"] == 0.0)
    }

    // MARK: - Cosine Similarity Tests

    @Test("Cosine similarity of identical vectors is 1.0")
    func cosineIdentical() {
        let v = FeatureVector(components: ["a": 1.0, "b": 2.0])
        let score = SimilarityScoring.score(v, v, metric: .cosine)
        #expect(abs(score - 1.0) < 0.001)
    }

    @Test("Cosine similarity of orthogonal vectors is 0.0")
    func cosineOrthogonal() {
        let a = FeatureVector(components: ["x": 1.0])
        let b = FeatureVector(components: ["y": 1.0])
        let score = SimilarityScoring.score(a, b, metric: .cosine)
        #expect(abs(score) < 0.001)
    }

    @Test("Cosine similarity known values")
    func cosineKnownValues() {
        let a = FeatureVector(components: ["a": 1.0, "b": 1.0])
        let b = FeatureVector(components: ["a": 1.0, "b": 0.0])
        let score = SimilarityScoring.score(a, b, metric: .cosine)
        // cos(45 degrees) is approximately 0.7071
        #expect(abs(score - 0.7071) < 0.01)
    }

    @Test("Cosine with empty vector returns 0")
    func cosineEmptyVector() {
        let a = FeatureVector(components: ["a": 1.0])
        let b = FeatureVector(components: [:])
        let score = SimilarityScoring.score(a, b, metric: .cosine)
        #expect(score == 0.0)
    }

    // MARK: - Jensen-Shannon Tests

    @Test("Jensen-Shannon of identical distributions is 1.0")
    func jsdIdentical() {
        let v = FeatureVector(components: ["a": 0.5, "b": 0.5])
        let score = SimilarityScoring.score(v, v, metric: .jensenShannon)
        #expect(abs(score - 1.0) < 0.001)
    }

    @Test("Jensen-Shannon of disjoint distributions is low")
    func jsdDisjoint() {
        let a = FeatureVector(components: ["x": 1.0])
        let b = FeatureVector(components: ["y": 1.0])
        let score = SimilarityScoring.score(a, b, metric: .jensenShannon)
        #expect(score < 0.5)
    }

    // MARK: - Spearman Rank Tests

    @Test("Spearman rank of identical ordering is 1.0")
    func spearmanIdentical() {
        let a = FeatureVector(components: ["a": 1.0, "b": 2.0, "c": 3.0])
        let score = SimilarityScoring.score(a, a, metric: .spearmanRank)
        #expect(abs(score - 1.0) < 0.001)
    }

    @Test("Spearman rank of reversed ordering is 0.0")
    func spearmanReversed() {
        let a = FeatureVector(components: ["a": 1.0, "b": 2.0, "c": 3.0])
        let b = FeatureVector(components: ["a": 3.0, "b": 2.0, "c": 1.0])
        let score = SimilarityScoring.score(a, b, metric: .spearmanRank)
        #expect(abs(score) < 0.01)
    }

    // MARK: - Rank Tests

    @Test("Rank returns results sorted by score descending")
    func rankOrdering() {
        let query = FeatureVector(components: ["a": 1.0, "b": 1.0])
        let candidates = [
            FeatureVector(components: ["a": 1.0]),                 // partial match
            FeatureVector(components: ["a": 1.0, "b": 1.0]),      // exact match
            FeatureVector(components: ["c": 1.0])                  // no match
        ]
        let ranked = SimilarityScoring.rank(
            query: query, candidates: candidates,
            metric: .cosine, threshold: 0.0, limit: 10
        )
        #expect(ranked.first?.index == 1)  // exact match first
    }

    @Test("Rank respects threshold")
    func rankThreshold() {
        let query = FeatureVector(components: ["a": 1.0])
        let candidates = [
            FeatureVector(components: ["a": 1.0]),  // perfect match
            FeatureVector(components: ["b": 1.0])   // no match
        ]
        let ranked = SimilarityScoring.rank(
            query: query, candidates: candidates,
            metric: .cosine, threshold: 0.5, limit: 10
        )
        #expect(ranked.count == 1)
        #expect(ranked.first?.index == 0)
    }

    @Test("Rank respects limit")
    func rankLimit() {
        let query = FeatureVector(components: ["a": 1.0, "b": 1.0, "c": 1.0])
        let candidates = (0..<10).map { i in
            FeatureVector(components: ["a": Double(i)])
        }
        let ranked = SimilarityScoring.rank(
            query: query, candidates: candidates,
            metric: .cosine, threshold: 0.0, limit: 3
        )
        #expect(ranked.count <= 3)
    }

    // MARK: - FileMetadataExtractor Tests

    @Test("Metadata extraction produces correct filename")
    func metadataFilename() {
        let extractor = FileMetadataExtractor()
        let meta = extractor.extract(relativePath: "docs/report.pdf", size: 1024)
        #expect(meta.filename == "report.pdf")
        #expect(meta.fileExtension == "pdf")
        #expect(meta.size == 1024)
    }

    @Test("Metadata extraction tokenizes camelCase")
    func metadataTokenization() {
        let extractor = FileMetadataExtractor()
        let meta = extractor.extract(relativePath: "MonthlyReport_2024.pdf", size: 500)
        let lowercaseKeywords = meta.keywords.map { $0.lowercased() }
        #expect(lowercaseKeywords.contains("monthly"))
        #expect(lowercaseKeywords.contains("report"))
    }

    @Test("Metadata extraction finds directory components")
    func metadataDirectories() {
        let extractor = FileMetadataExtractor()
        let meta = extractor.extract(relativePath: "reports/finance/annual.xlsx", size: 2048)
        #expect(meta.directoryComponents == ["reports", "finance"])
    }

    // MARK: - FileKnowledgeGraph Tests

    @Test("Knowledge graph adds and queries triples")
    func knowledgeGraphBasic() {
        let graph = FileKnowledgeGraph()
        let triple = RDFTriple(
            subject: .uri("file:test"),
            predicate: RDFPredicate.hasName,
            object: .literal("test.txt", .string)
        )
        graph.add(triple)
        let results = graph.query(pattern: TriplePattern(
            subject: .uri("file:test"), predicate: nil, object: nil
        ))
        #expect(results.count == 1)
        #expect(results.first?.predicate == RDFPredicate.hasName)
    }

    @Test("Knowledge graph addFileMetadata generates triples")
    func knowledgeGraphMetadata() {
        let graph = FileKnowledgeGraph()
        let meta = FileMetadata(
            relativePath: "docs/test.txt",
            filename: "test.txt",
            fileExtension: "txt",
            size: 100,
            keywords: ["test", "document"],
            entities: [],
            directoryComponents: ["docs"]
        )
        graph.addFileMetadata(meta)
        // Should have at least: hasName, hasFileType, hasSize, hasKeyword x2, inDirectory
        let allTriples = graph.query(pattern: TriplePattern(
            subject: .uri("file:\(meta.id)"), predicate: nil, object: nil
        ))
        #expect(allTriples.count >= 6)
    }

    @Test("Knowledge graph BGP evaluation")
    func knowledgeGraphBGP() {
        let graph = FileKnowledgeGraph()
        let meta = FileMetadata(
            relativePath: "reports/annual.pdf",
            filename: "annual.pdf",
            fileExtension: "pdf",
            size: 5000,
            keywords: ["annual", "report"],
            entities: [],
            directoryComponents: ["reports"]
        )
        graph.addFileMetadata(meta)

        // Query: files that have keyword "annual" AND are in directory "reports"
        let patterns = [
            TriplePattern(subject: nil, predicate: RDFPredicate.hasKeyword, object: .literal("annual", .string)),
            TriplePattern(subject: nil, predicate: RDFPredicate.inDirectory, object: .literal("reports", .string))
        ]
        let results = graph.queryBGP(patterns: patterns)
        #expect(results.count == 1)
    }

    @Test("Knowledge graph clear removes all triples")
    func knowledgeGraphClear() {
        let graph = FileKnowledgeGraph()
        graph.add(RDFTriple(
            subject: .uri("file:1"),
            predicate: RDFPredicate.hasName,
            object: .literal("test", .string)
        ))
        graph.clear()
        let results = graph.query(pattern: TriplePattern(subject: nil, predicate: nil, object: nil))
        #expect(results.isEmpty)
    }
}
