import Testing
import Foundation
@testable import AlpineApp

@Suite("RestApiTransport Tests")
struct RestApiTransportTests {

    let transport: RestApiTransport

    init() {
        URLProtocol.registerClass(MockURLProtocol.self)
        let service = createMockApiService()
        transport = RestApiTransport(apiService: service)
    }

    // MARK: - startQuery

    @Test("startQuery delegates to API service and returns QueryResponse")
    func testStartQuery() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/queries") == true)

            let body = try JSONDecoder().decode(QueryRequest.self, from: request.httpBody!)
            #expect(body.queryString == "*.txt")

            return try mockResponse(json: ["queryId": 555])
        }

        let request = QueryRequest(queryString: "*.txt")
        let response = try await transport.startQuery(request)
        #expect(response.queryId == 555)
    }

    @Test("startQuery passes all request fields")
    func testStartQueryFullRequest() async throws {
        MockURLProtocol.requestHandler = { request in
            let body = try JSONDecoder().decode(QueryRequest.self, from: request.httpBody!)
            #expect(body.queryString == "music/*.mp3")
            #expect(body.groupName == "media")
            #expect(body.autoHaltLimit == 300)
            #expect(body.peerDescMax == 15)
            return try mockResponse(json: ["queryId": 100])
        }

        let request = QueryRequest(
            queryString: "music/*.mp3",
            groupName: "media",
            autoHaltLimit: 300,
            peerDescMax: 15
        )
        let response = try await transport.startQuery(request)
        #expect(response.queryId == 100)
    }

    // MARK: - getQueryStatus

    @Test("getQueryStatus delegates to API service")
    func testGetQueryStatus() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.contains("/queries/555") == true)
            return try mockResponse(json: [
                "inProgress": false,
                "totalPeers": 3,
                "peersQueried": 3,
                "numPeerResponses": 2,
                "totalHits": 7
            ])
        }

        let status = try await transport.getQueryStatus(555)
        #expect(status.inProgress == false)
        #expect(status.totalPeers == 3)
        #expect(status.peersQueried == 3)
        #expect(status.numPeerResponses == 2)
        #expect(status.totalHits == 7)
    }

    @Test("getQueryStatus reflects in-progress query")
    func testGetQueryStatusInProgress() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: [
                "inProgress": true,
                "totalPeers": 50,
                "peersQueried": 10,
                "numPeerResponses": 5,
                "totalHits": 25
            ])
        }

        let status = try await transport.getQueryStatus(42)
        #expect(status.inProgress == true)
        #expect(status.peersQueried == 10)
    }

    // MARK: - getQueryResults

    @Test("getQueryResults delegates to API service")
    func testGetQueryResults() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.path.hasSuffix("/queries/555/results") == true)
            return try mockResponse(json: ["peers": []])
        }

        let results = try await transport.getQueryResults(555)
        #expect(results.peers.isEmpty)
    }

    @Test("getQueryResults with populated data")
    func testGetQueryResultsWithData() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: [
                "peers": [
                    [
                        "peerId": 10,
                        "resources": [
                            [
                                "resourceId": 100,
                                "size": 8192,
                                "locators": ["http://peer10/file"],
                                "description": "document.pdf"
                            ]
                        ]
                    ]
                ]
            ])
        }

        let results = try await transport.getQueryResults(42)
        #expect(results.peers.count == 1)
        #expect(results.peers[0].peerId == 10)
        #expect(results.peers[0].resources[0].description == "document.pdf")
    }

    // MARK: - cancelQuery

    @Test("cancelQuery delegates to API service with DELETE")
    func testCancelQuery() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            #expect(request.url?.path.contains("/queries/555") == true)
            return try mockResponse(json: ["success": true])
        }

        try await transport.cancelQuery(555)
    }

    // MARK: - Error Handling

    @Test("transport propagates HTTP errors")
    func testErrorPropagation() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 500, json: ["error": "Internal server error"])
        }

        do {
            _ = try await transport.getQueryStatus(99)
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, _) = error {
                #expect(code == 500)
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
    }

    @Test("transport propagates decoding errors")
    func testDecodingErrorPropagation() async throws {
        MockURLProtocol.requestHandler = { _ in
            let data = "bad data".data(using: .utf8)!
            let response = HTTPURLResponse(
                url: URL(string: "http://localhost:8080/api/v1/queries/1")!,
                statusCode: 200,
                httpVersion: "HTTP/1.1",
                headerFields: nil
            )!
            return (response, data)
        }

        do {
            _ = try await transport.getQueryStatus(1)
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .decodingFailed = error {
                // Expected
            } else {
                Issue.record("Expected decodingFailed, got \(error)")
            }
        }
    }

    // MARK: - QueryTransport Protocol Conformance

    @Test("RestApiTransport conforms to QueryTransport protocol")
    func testProtocolConformance() async throws {
        // Verify the transport can be used where QueryTransport is expected
        let _: any QueryTransport = transport

        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: ["queryId": 1])
        }

        let queryTransport: any QueryTransport = transport
        let response = try await queryTransport.startQuery(QueryRequest(queryString: "test"))
        #expect(response.queryId == 1)
    }
}
