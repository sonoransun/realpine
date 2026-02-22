import Foundation
import Observation

/// Persists recent search queries as JSON in Application Support.
@Observable
final class SearchHistoryStore {
    struct SearchHistoryEntry: Codable, Identifiable {
        let id: UUID
        let query: String
        let mode: String
        let timestamp: Date
        var resultCount: Int?

        init(id: UUID = UUID(), query: String, mode: String, timestamp: Date = Date(), resultCount: Int? = nil) {
            self.id = id
            self.query = query
            self.mode = mode
            self.timestamp = timestamp
            self.resultCount = resultCount
        }
    }

    private(set) var entries: [SearchHistoryEntry] = []

    private static let maxEntries = 50
    private let fileURL: URL

    init() {
        let appSupport = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first!
        let directory = appSupport.appendingPathComponent("AlpineApp", isDirectory: true)

        if !FileManager.default.fileExists(atPath: directory.path) {
            try? FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        }

        self.fileURL = directory.appendingPathComponent("search_history.json")
        load()
    }

    /// Convenience method to add a search entry by query string, mode, and optional result count.
    func add(query: String, mode: String, resultCount: Int? = nil) {
        let entry = SearchHistoryEntry(query: query, mode: mode, resultCount: resultCount)
        add(entry)
    }

    /// Adds a new search entry, keeping the list capped at the maximum and sorted newest first.
    func add(_ entry: SearchHistoryEntry) {
        entries.insert(entry, at: 0)
        if entries.count > Self.maxEntries {
            entries = Array(entries.prefix(Self.maxEntries))
        }
        save()
    }

    /// Removes a single entry by its identifier.
    func remove(id: UUID) {
        entries.removeAll { $0.id == id }
        save()
    }

    /// Clears all stored history entries.
    func clear() {
        entries.removeAll()
        save()
    }

    // MARK: - Persistence

    private func load() {
        guard FileManager.default.fileExists(atPath: fileURL.path) else { return }
        do {
            let data = try Data(contentsOf: fileURL)
            let decoder = JSONDecoder()
            decoder.dateDecodingStrategy = .iso8601
            entries = try decoder.decode([SearchHistoryEntry].self, from: data)
            entries.sort { $0.timestamp > $1.timestamp }
        } catch {
            entries = []
        }
    }

    private func save() {
        do {
            let encoder = JSONEncoder()
            encoder.dateEncodingStrategy = .iso8601
            encoder.outputFormatting = .prettyPrinted
            let data = try encoder.encode(entries)
            try data.write(to: fileURL, options: .atomic)
        } catch {
            // Silently fail; persistence is best-effort.
        }
    }
}
