import Foundation

/// Transport implementation that delegates query operations to AlpineApiService REST endpoints.
/// Replaces the former JsonRpcTransport that used JSON-RPC.
final class RestApiTransport: QueryTransport {
    private let apiService: AlpineApiService

    init(apiService: AlpineApiService) {
        self.apiService = apiService
    }

    func startQuery(_ request: QueryRequest) async throws -> QueryResponse {
        try await apiService.startQuery(request)
    }

    func getQueryStatus(_ queryId: Int64) async throws -> QueryStatusResponse {
        try await apiService.getQueryStatus(queryId: queryId)
    }

    func getQueryResults(_ queryId: Int64) async throws -> QueryResultsResponse {
        try await apiService.getQueryResults(queryId: queryId)
    }

    func cancelQuery(_ queryId: Int64) async throws {
        _ = try await apiService.cancelQuery(queryId: queryId)
    }

    func shutdown() {
        apiService.shutdown()
    }
}
