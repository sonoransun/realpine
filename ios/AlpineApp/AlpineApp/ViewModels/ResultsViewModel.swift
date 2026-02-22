import Foundation
import Observation

@Observable
final class ResultsViewModel {
    var queryStatus: QueryStatusResponse?
    var results: [PeerResources] = []
    var isLoading = true
    var error: String?
    var sortOrder: ResultSortOrder = .relevance

    var flattenedResults: [ResourceDesc] {
        let allResources = results.flatMap(\.resources)
        switch sortOrder {
        case .relevance:
            return allResources.sorted { ($0.score ?? 0) > ($1.score ?? 0) }
        case .name:
            return allResources.sorted { $0.description.localizedCaseInsensitiveCompare($1.description) == .orderedAscending }
        case .size:
            return allResources.sorted { $0.size > $1.size }
        }
    }

    var hasScores: Bool {
        results.contains { peer in
            peer.resources.contains { $0.score != nil }
        }
    }

    var refinements: [QueryRefinement] {
        results.flatMap(\.resources).compactMap(\.refinements).flatMap { $0 }
    }

    private let queryId: Int64
    private let transport: QueryTransport
    private var pollingTask: Task<Void, Never>?

    init(queryId: Int64, settings: SettingsStore, secureStorage: SecureStorage) {
        self.queryId = queryId
        self.transport = TransportProvider.createTransport(
            settings: settings,
            secureStorage: secureStorage
        )
        startPolling()
    }

    private func startPolling() {
        pollingTask = Task { [weak self] in
            guard let self else { return }
            while !Task.isCancelled {
                do {
                    let status = try await transport.getQueryStatus(queryId)
                    let resultsResponse = try await transport.getQueryResults(queryId)
                    await MainActor.run {
                        self.queryStatus = status
                        self.results = resultsResponse.peers
                        self.isLoading = false
                    }
                    if !status.inProgress { break }
                } catch {
                    await MainActor.run {
                        self.error = ErrorMessages.userFriendly(from: error)
                        self.isLoading = false
                    }
                    break
                }
                try? await Task.sleep(for: .seconds(2))
            }
        }
    }

    func cancelQuery() {
        Task {
            try? await transport.cancelQuery(queryId)
            stopPolling()
        }
    }

    func stopPolling() {
        pollingTask?.cancel()
        pollingTask = nil
    }

    deinit {
        pollingTask?.cancel()
        transport.shutdown()
    }
}
