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

    private let settings: SettingsStore
    private let secureStorage: SecureStorage

    init(settings: SettingsStore, secureStorage: SecureStorage) {
        self.settings = settings
        self.secureStorage = secureStorage
        self.transportMode = settings.transportMode
    }

    func search() async -> Int64? {
        let sanitized = InputValidator.sanitizeQuery(queryString)
        guard !sanitized.isEmpty else {
            error = "Please enter a search query"
            return nil
        }

        isLoading = true
        error = nil

        do {
            let transport = TransportProvider.createTransport(
                settings: settings,
                secureStorage: secureStorage
            )
            let request = QueryRequest(
                queryString: sanitized,
                groupName: groupName,
                autoHaltLimit: Int64(autoHaltLimit) ?? 100,
                peerDescMax: Int64(peerDescMax) ?? 50
            )
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
}
