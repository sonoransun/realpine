import Foundation

protocol QueryTransport: Sendable {
    func startQuery(_ request: QueryRequest) async throws -> QueryResponse
    func getQueryStatus(_ queryId: Int64) async throws -> QueryStatusResponse
    func getQueryResults(_ queryId: Int64) async throws -> QueryResultsResponse
    func cancelQuery(_ queryId: Int64) async throws
    func shutdown()
}
