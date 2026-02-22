import Foundation
import Observation

@Observable
final class ResultsViewModel {
    var queryStatus: QueryStatusResponse?
    var results: [PeerResources] = []
    var isLoading = true
    var error: String?

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
