import Foundation
import Observation

@Observable
final class SearchViewModel {
    var queryString: String = ""
    var groupName: String = ""
    var autoHaltLimit: String = "100"
    var peerDescMax: String = "50"
    var isLoading = false
    var error: String?
    var transportMode: TransportMode
    var searchMode: SearchMode = .keyword
    var entityFilters: Set<EntityType> = []
    var similarityThreshold: Float = 0.5
    var sparqlQuery: String = ""
    var contentCategoryFilters: Set<ContentCategory> = []
    var languageFilters: Set<String> = []
    var signalWeights = SignalWeights.default

    private let settings: SettingsStore
    private let secureStorage: SecureStorage
    var searchHistory: SearchHistoryStore

    init(settings: SettingsStore, secureStorage: SecureStorage, searchHistory: SearchHistoryStore) {
        self.settings = settings
        self.secureStorage = secureStorage
        self.searchHistory = searchHistory
        self.transportMode = settings.transportMode
    }

    var queryPreview: String {
        switch searchMode {
        case .keyword:
            return "Glob + keyword search: \(queryString)"
        case .semantic:
            return "Semantic similarity (\(Int(similarityThreshold * 100))%) search: \(queryString)"
        case .entity:
            let types = entityFilters.map(\.rawValue).joined(separator: ", ")
            return "Entity search\(types.isEmpty ? "" : " [\(types)]"): \(queryString)"
        case .sparql:
            return "SPARQL query (\(sparqlQuery.count) chars)"
        case .multiModal:
            var parts: [String] = ["Multi-modal: \(queryString)"]
            if !contentCategoryFilters.isEmpty {
                parts.append("categories: \(contentCategoryFilters.map(\.rawValue).joined(separator: ","))")
            }
            if !languageFilters.isEmpty {
                parts.append("languages: \(languageFilters.joined(separator: ","))")
            }
            return parts.joined(separator: " | ")
        }
    }

    func search() async -> Int64? {
        // In SPARQL mode, the query string comes from sparqlQuery
        let effectiveQuery = searchMode == .sparql ? "sparql" : queryString
        let sanitized = InputValidator.sanitizeQuery(effectiveQuery)
        guard !sanitized.isEmpty else {
            error = "Please enter a search query"
            return nil
        }

        // Record the search in history
        searchHistory.add(query: sanitized, mode: searchMode.rawValue, resultCount: nil)

        isLoading = true
        error = nil

        do {
            let transport = TransportProvider.createTransport(
                settings: settings,
                secureStorage: secureStorage
            )
            var request = QueryRequest(
                queryString: sanitized,
                groupName: groupName,
                autoHaltLimit: Int64(autoHaltLimit) ?? 100,
                peerDescMax: Int64(peerDescMax) ?? 50
            )

            // Set enhanced search fields
            request.searchMode = searchMode
            request.protocolVersion = searchMode == .multiModal ? 3 : 2

            switch searchMode {
            case .keyword:
                break
            case .semantic:
                request.similarityMetric = .cosine
                request.similarityThreshold = similarityThreshold
            case .entity:
                if !entityFilters.isEmpty {
                    request.entityFilters = entityFilters.map { type in
                        EntityFilter(text: queryString, entityType: type, minConfidence: 0.5)
                    }
                } else {
                    request.entityFilters = [EntityFilter(text: queryString, entityType: nil, minConfidence: 0.3)]
                }
            case .sparql:
                request.sparqlPatterns = parseSparqlPatterns(sparqlQuery)
            case .multiModal:
                request.similarityMetric = .cosine
                request.similarityThreshold = similarityThreshold
                if !contentCategoryFilters.isEmpty {
                    request.contentCategories = Array(contentCategoryFilters)
                }
                if !languageFilters.isEmpty {
                    request.languages = Array(languageFilters)
                }
                request.signalWeights = signalWeights
            }

            let response = try await transport.startQuery(request)
            isLoading = false
            return response.queryId
        } catch {
            self.error = ErrorMessages.userFriendly(from: error)
            isLoading = false
            return nil
        }
    }

    func clearError() { error = nil }

    func applyRefinement(_ refinement: QueryRefinement) {
        switch refinement.refinementType {
        case .addCategoryFilter:
            if let cat = refinement.parameters.contentCategory {
                contentCategoryFilters.insert(cat)
            }
        case .addLanguageFilter:
            if let lang = refinement.parameters.languageCode {
                languageFilters.insert(lang)
            }
        case .addEntityFilter:
            if let type = refinement.parameters.entityType {
                entityFilters.insert(type)
            }
        case .addKeywordFilter:
            if let kw = refinement.parameters.keyword {
                queryString += " \(kw)"
            }
        case .adjustThreshold:
            if let t = refinement.parameters.threshold {
                similarityThreshold = t
            }
        case .removeAmbiguity:
            if let kw = refinement.parameters.keyword {
                queryString = kw
            }
        }
    }

    // MARK: - Private Helpers

    private func parseSparqlPatterns(_ query: String) -> [CodableTriplePattern] {
        // Simple pattern parser: each line is "subject predicate object"
        // Use ? prefix for variables (treated as nil/wildcard)
        query.components(separatedBy: .newlines)
            .map { $0.trimmingCharacters(in: .whitespaces) }
            .filter { !$0.isEmpty }
            .compactMap { line in
                let parts = line.components(separatedBy: .whitespaces).filter { !$0.isEmpty }
                guard parts.count >= 2 else { return nil }
                let subject = parts[0].hasPrefix("?") ? nil : parts[0]
                let predicate = parts[1].hasPrefix("?") ? nil : parts[1]
                let object = parts.count > 2 ? (parts[2].hasPrefix("?") ? nil : parts[2...].joined(separator: " ")) : nil
                return CodableTriplePattern(subject: subject, predicate: predicate, object: object)
            }
    }
}
