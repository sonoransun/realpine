import Foundation

/// Wraps AlpineRpcService to conform to the QueryTransport protocol,
/// providing query operations over JSON-RPC to an Alpine REST Bridge.
final class JsonRpcTransport: QueryTransport {
    private let rpcService: AlpineRpcService

    init(rpcService: AlpineRpcService) {
        self.rpcService = rpcService
    }

    func startQuery(_ request: QueryRequest) async throws -> QueryResponse {
        let queryId = try await rpcService.startQuery(
            queryString: request.queryString,
            groupName: request.groupName,
            autoHaltLimit: request.autoHaltLimit,
            peerDescMax: request.peerDescMax
        )
        return QueryResponse(queryId: queryId)
    }

    func getQueryStatus(_ queryId: Int64) async throws -> QueryStatusResponse {
        try await rpcService.getQueryStatus(queryId: queryId)
    }

    func getQueryResults(_ queryId: Int64) async throws -> QueryResultsResponse {
        try await rpcService.getQueryResults(queryId: queryId)
    }

    func cancelQuery(_ queryId: Int64) async throws {
        _ = try await rpcService.cancelQuery(queryId: queryId)
    }

    func shutdown() {
        rpcService.shutdown()
    }
}
