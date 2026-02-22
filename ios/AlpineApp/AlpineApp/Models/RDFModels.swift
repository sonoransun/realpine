import Foundation
import os

private let logger = Logger(subsystem: "com.sonoranpub.AlpineApp", category: "RDFModels")

// MARK: - RDF Core Types

enum RDFLiteralType: String, Codable, Sendable {
    case string, integer, double, dateTime, boolean
}

enum RDFNode: Codable, Sendable, Hashable {
    case uri(String)
    case literal(String, RDFLiteralType)
    case blank(String)
}

enum RDFPredicate {
    static let hasName = "alpine:hasName"
    static let hasKeyword = "alpine:hasKeyword"
    static let hasEntity = "alpine:hasEntity"
    static let hasEntityType = "alpine:hasEntityType"
    static let hasFileType = "alpine:hasFileType"
    static let hasSize = "alpine:hasSize"
    static let inDirectory = "alpine:inDirectory"
    static let hasLocator = "alpine:hasLocator"
    static let hasConfidence = "alpine:hasConfidence"
    static let hasLanguage = "alpine:hasLanguage"
    static let hasContentCategory = "alpine:hasContentCategory"
    static let hasMediaWidth = "alpine:hasMediaWidth"
    static let hasMediaHeight = "alpine:hasMediaHeight"
    static let hasMediaDuration = "alpine:hasMediaDuration"
}

struct RDFTriple: Codable, Sendable, Identifiable, Hashable {
    let id: UUID
    let subject: RDFNode
    let predicate: String
    let object: RDFNode

    init(id: UUID = UUID(), subject: RDFNode, predicate: String, object: RDFNode) {
        self.id = id
        self.subject = subject
        self.predicate = predicate
        self.object = object
    }
}

struct TriplePattern: Sendable {
    let subject: RDFNode?
    let predicate: String?
    let object: RDFNode?

    func matches(_ triple: RDFTriple) -> Bool {
        if let s = subject, s != triple.subject { return false }
        if let p = predicate, p != triple.predicate { return false }
        if let o = object, o != triple.object { return false }
        return true
    }
}

// MARK: - FileKnowledgeGraph

/// Thread-safe in-memory RDF triple store with three-way indexing for fast pattern queries.
final class FileKnowledgeGraph: @unchecked Sendable {
    private struct StoreState {
        var triples: [RDFTriple] = []
        var subjectIndex: [RDFNode: [Int]] = [:]
        var predicateIndex: [String: [Int]] = [:]
        var objectIndex: [RDFNode: [Int]] = [:]
    }

    private let store = OSAllocatedUnfairLock(initialState: StoreState())

    // MARK: - Mutation

    func add(_ triple: RDFTriple) {
        store.withLock { state in
            let idx = state.triples.count
            state.triples.append(triple)

            state.subjectIndex[triple.subject, default: []].append(idx)
            state.predicateIndex[triple.predicate, default: []].append(idx)
            state.objectIndex[triple.object, default: []].append(idx)
        }
    }

    /// Generate and store RDF triples that describe a file's metadata.
    func addFileMetadata(_ metadata: FileMetadata) {
        logger.debug("Adding metadata for file: \(metadata.relativePath)")
        let subject = RDFNode.uri("file:\(metadata.id)")

        // hasName
        add(RDFTriple(
            subject: subject,
            predicate: RDFPredicate.hasName,
            object: .literal(metadata.filename, .string)
        ))

        // hasKeyword for each keyword
        for keyword in metadata.keywords {
            add(RDFTriple(
                subject: subject,
                predicate: RDFPredicate.hasKeyword,
                object: .literal(keyword, .string)
            ))
        }

        // hasEntity + blank node with type and confidence for each entity
        for entity in metadata.entities {
            add(RDFTriple(
                subject: subject,
                predicate: RDFPredicate.hasEntity,
                object: .literal(entity.text, .string)
            ))

            let blankId = "_:entity_\(entity.id.uuidString)"
            let blankNode = RDFNode.blank(blankId)

            add(RDFTriple(
                subject: blankNode,
                predicate: RDFPredicate.hasEntityType,
                object: .literal(entity.entityType.rawValue, .string)
            ))

            add(RDFTriple(
                subject: blankNode,
                predicate: RDFPredicate.hasConfidence,
                object: .literal(String(entity.confidence), .double)
            ))
        }

        // hasFileType
        add(RDFTriple(
            subject: subject,
            predicate: RDFPredicate.hasFileType,
            object: .literal(metadata.fileExtension, .string)
        ))

        // hasSize
        add(RDFTriple(
            subject: subject,
            predicate: RDFPredicate.hasSize,
            object: .literal(String(metadata.size), .integer)
        ))

        // inDirectory for each directory component
        for component in metadata.directoryComponents {
            add(RDFTriple(
                subject: subject,
                predicate: RDFPredicate.inDirectory,
                object: .literal(component, .string)
            ))
        }

        // Language
        if let lang = metadata.language {
            add(RDFTriple(
                subject: subject,
                predicate: RDFPredicate.hasLanguage,
                object: .literal(lang.languageCode, .string)
            ))
        }
        // Content category
        if let cat = metadata.contentCategory {
            add(RDFTriple(
                subject: subject,
                predicate: RDFPredicate.hasContentCategory,
                object: .literal(cat.rawValue, .string)
            ))
        }
        // Media dimensions
        if let media = metadata.mediaMetadata {
            if let w = media.width {
                add(RDFTriple(
                    subject: subject,
                    predicate: RDFPredicate.hasMediaWidth,
                    object: .literal(String(w), .integer)
                ))
            }
            if let h = media.height {
                add(RDFTriple(
                    subject: subject,
                    predicate: RDFPredicate.hasMediaHeight,
                    object: .literal(String(h), .integer)
                ))
            }
            if let dur = media.durationSeconds {
                add(RDFTriple(
                    subject: subject,
                    predicate: RDFPredicate.hasMediaDuration,
                    object: .literal(String(dur), .double)
                ))
            }
        }
    }

    // MARK: - Query

    /// Query triples matching a pattern, using the most selective index first.
    func query(pattern: TriplePattern) -> [RDFTriple] {
        store.withLock { state in
            Self.queryState(state, pattern: pattern)
        }
    }

    /// Internal query on state without locking (caller must hold the lock).
    private static func queryState(_ state: StoreState, pattern: TriplePattern) -> [RDFTriple] {
        // Determine which index yields the smallest candidate set
        var candidateIndices: [Int]?

        if let s = pattern.subject {
            let indices = state.subjectIndex[s] ?? []
            if candidateIndices == nil || indices.count < candidateIndices!.count {
                candidateIndices = indices
            }
        }

        if let p = pattern.predicate {
            let indices = state.predicateIndex[p] ?? []
            if candidateIndices == nil || indices.count < candidateIndices!.count {
                candidateIndices = indices
            }
        }

        if let o = pattern.object {
            let indices = state.objectIndex[o] ?? []
            if candidateIndices == nil || indices.count < candidateIndices!.count {
                candidateIndices = indices
            }
        }

        // If no constraints, return all triples
        guard let indices = candidateIndices else {
            return state.triples
        }

        // Filter candidates through the full pattern
        var results: [RDFTriple] = []
        results.reserveCapacity(indices.count)
        for idx in indices {
            let triple = state.triples[idx]
            if pattern.matches(triple) {
                results.append(triple)
            }
        }
        return results
    }

    /// Basic Graph Pattern: evaluate multiple patterns and return the set of subject
    /// nodes that satisfy all of them.
    func queryBGP(patterns: [TriplePattern]) -> Set<RDFNode> {
        store.withLock { state in
            guard let first = patterns.first else { return [] }

            // Evaluate the first pattern to get initial candidate subjects
            var candidates: Set<RDFNode> = Set(
                Self.queryState(state, pattern: first).map { $0.subject }
            )

            // For each subsequent pattern, intersect with matching subjects
            for i in 1..<patterns.count {
                guard !candidates.isEmpty else { break }

                let matches = Self.queryState(state, pattern: patterns[i])
                let matchingSubjects = Set(matches.map { $0.subject })
                candidates = candidates.intersection(matchingSubjects)
            }

            return candidates
        }
    }

    // MARK: - Clear

    func clear() {
        store.withLock { state in
            state.triples.removeAll()
            state.subjectIndex.removeAll()
            state.predicateIndex.removeAll()
            state.objectIndex.removeAll()
        }
        logger.debug("Knowledge graph cleared")
    }
}
