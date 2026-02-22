import Testing
import Foundation
@testable import AlpineApp

@Suite("RestApiClient Tests")
struct RestApiClientTests {

    init() {
        URLProtocol.registerClass(MockURLProtocol.self)
    }

    // MARK: - GET Requests

    @Test("GET request decodes response correctly")
    func testGetRequest() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.contains("/status") == true)
            #expect(request.value(forHTTPHeaderField: "Accept") == "application/json")
            return try mockResponse(json: ["status": "running", "version": "1.0.0"])
        }

        let client = createMockClient()
        let status: ServerStatus = try await client.get("/status")
        #expect(status.status == "running")
        #expect(status.version == "1.0.0")
        await client.shutdown()
    }

    @Test("GET request with query parameters")
    func testGetWithQueryParams() async throws {
        MockURLProtocol.requestHandler = { request in
            let url = request.url!
            let components = URLComponents(url: url, resolvingAgainstBaseURL: false)!
            let ip = components.queryItems?.first(where: { $0.name == "ip" })?.value
            let port = components.queryItems?.first(where: { $0.name == "port" })?.value
            #expect(ip == "192.168.1.1")
            #expect(port == "9090")
            return try mockResponse(json: ["peerId": 42])
        }

        let client = createMockClient()
        let response: PeerIdResponse = try await client.get(
            "/peers/lookup",
            queryItems: [
                URLQueryItem(name: "ip", value: "192.168.1.1"),
                URLQueryItem(name: "port", value: "9090")
            ]
        )
        #expect(response.peerId == 42)
        await client.shutdown()
    }

    @Test("GET void request validates success without decoding body")
    func testGetVoidRequest() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            return try mockResponse(json: [:])
        }

        let client = createMockClient()
        try await client.get("/health")
        await client.shutdown()
    }

    // MARK: - POST Requests

    @Test("POST request sends body and decodes response")
    func testPostWithBody() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.value(forHTTPHeaderField: "Content-Type") == "application/json")

            let body = try JSONDecoder().decode(QueryRequest.self, from: request.httpBody!)
            #expect(body.queryString == "*.mp3")
            #expect(body.autoHaltLimit == 50)

            return try mockResponse(json: ["queryId": 12345])
        }

        let client = createMockClient()
        let request = QueryRequest(queryString: "*.mp3", autoHaltLimit: 50)
        let response: QueryResponse = try await client.post("/queries", body: request)
        #expect(response.queryId == 12345)
        await client.shutdown()
    }

    @Test("POST request with body, void response")
    func testPostBodyVoidResponse() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.httpBody != nil)
            return try mockResponse(json: [:])
        }

        let client = createMockClient()
        let body = HostFilterRequest(ipAddress: "10.0.0.1")
        try await client.post("/filters/hosts/exclude", body: body)
        await client.shutdown()
    }

    @Test("POST request with no body, returning response")
    func testPostNoBodyWithResponse() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.httpBody == nil)
            return try mockResponse(json: ["success": true])
        }

        let client = createMockClient()
        let response: SuccessResponse = try await client.post("/peers/5/ping")
        #expect(response.success == true)
        await client.shutdown()
    }

    @Test("POST request with no body, void response")
    func testPostNoBodyVoidResponse() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            return try mockResponse(json: [:])
        }

        let client = createMockClient()
        try await client.post("/action")
        await client.shutdown()
    }

    // MARK: - PUT Requests

    @Test("PUT request with body and response")
    func testPutWithBody() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.value(forHTTPHeaderField: "Content-Type") == "application/json")
            return try mockResponse(json: ["success": true])
        }

        let client = createMockClient()
        let body = AddPeerRequest(ipAddress: "10.0.0.1", port: 8080)
        let response: SuccessResponse = try await client.put("/peers/1", body: body)
        #expect(response.success == true)
        await client.shutdown()
    }

    @Test("PUT request with no body, returning response")
    func testPutNoBody() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.contains("/queries/123/pause") == true)
            return try mockResponse(json: ["success": true])
        }

        let client = createMockClient()
        let response: SuccessResponse = try await client.put("/queries/123/pause")
        #expect(response.success == true)
        await client.shutdown()
    }

    @Test("PUT request with no body, void response")
    func testPutNoBodyVoid() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            return try mockResponse(json: [:])
        }

        let client = createMockClient()
        try await client.put("/queries/123/pause")
        await client.shutdown()
    }

    // MARK: - DELETE Requests

    @Test("DELETE request returning decoded response")
    func testDeleteRequest() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            return try mockResponse(json: ["success": true])
        }

        let client = createMockClient()
        let response: SuccessResponse = try await client.delete("/queries/456")
        #expect(response.success == true)
        await client.shutdown()
    }

    @Test("DELETE request with void response")
    func testDeleteVoidRequest() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            return try mockResponse(json: [:])
        }

        let client = createMockClient()
        try await client.delete("/peers/99")
        await client.shutdown()
    }

    // MARK: - Authentication

    @Test("Bearer token authentication header is set")
    func testAuthHeader() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.value(forHTTPHeaderField: "Authorization") == "Bearer test-api-key-123")
            return try mockResponse(json: ["status": "running", "version": "1.0"])
        }

        let client = RestApiClient(
            baseURL: URL(string: "http://localhost:8080/api/v1")!,
            apiKey: "test-api-key-123"
        )
        let _: ServerStatus = try await client.get("/status")
        await client.shutdown()
    }

    @Test("No auth header when API key is empty")
    func testNoAuthHeaderWhenEmpty() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.value(forHTTPHeaderField: "Authorization") == nil)
            return try mockResponse(json: ["status": "running", "version": "1.0"])
        }

        let client = createMockClient()
        let _: ServerStatus = try await client.get("/status")
        await client.shutdown()
    }

    @Test("Accept header is always application/json")
    func testAcceptHeader() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.value(forHTTPHeaderField: "Accept") == "application/json")
            return try mockResponse(json: ["success": true])
        }

        let client = createMockClient()
        let _: SuccessResponse = try await client.post("/test")
        await client.shutdown()
    }

    // MARK: - Error Handling

    @Test("HTTP 400 throws httpError")
    func testHttp400() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 400, json: ["error": "Bad request"])
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/bad")
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, _) = error {
                #expect(code == 400)
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
        await client.shutdown()
    }

    @Test("HTTP 401 throws httpError")
    func testHttp401() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 401, json: ["error": "Unauthorized"])
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/protected")
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, _) = error {
                #expect(code == 401)
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
        await client.shutdown()
    }

    @Test("HTTP 404 throws httpError")
    func testHttp404() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 404, json: ["error": "Not found"])
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/nonexistent")
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, _) = error {
                #expect(code == 404)
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
        await client.shutdown()
    }

    @Test("HTTP 500 throws httpError")
    func testHttp500() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 500, json: ["error": "Internal server error"])
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/status")
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, _) = error {
                #expect(code == 500)
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
        await client.shutdown()
    }

    @Test("HTTP 503 throws httpError")
    func testHttp503() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 503, json: ["error": "Service unavailable"])
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/status")
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, let msg) = error {
                #expect(code == 503)
                #expect(msg.contains("Service unavailable"))
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
        await client.shutdown()
    }

    @Test("Invalid JSON throws decodingFailed")
    func testDecodingError() async throws {
        MockURLProtocol.requestHandler = { _ in
            let data = "not json".data(using: .utf8)!
            let response = HTTPURLResponse(
                url: URL(string: "http://localhost:8080/api/v1/status")!,
                statusCode: 200,
                httpVersion: "HTTP/1.1",
                headerFields: nil
            )!
            return (response, data)
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/status")
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .decodingFailed = error {
                // Expected
            } else {
                Issue.record("Expected decodingFailed, got \(error)")
            }
        }
        await client.shutdown()
    }

    @Test("Mismatched JSON structure throws decodingFailed")
    func testMismatchedJsonStructure() async throws {
        MockURLProtocol.requestHandler = { _ in
            // Return valid JSON that doesn't match the expected type
            try mockResponse(json: ["unexpectedField": 123])
        }

        let client = createMockClient()
        do {
            let _: ServerStatus = try await client.get("/status")
            Issue.record("Expected decoding error")
        } catch let error as ApiError {
            if case .decodingFailed = error {
                // Expected: ServerStatus requires "status" and "version" fields
            } else {
                Issue.record("Expected decodingFailed, got \(error)")
            }
        }
        await client.shutdown()
    }

    // MARK: - URL Construction

    @Test("Base URL is correctly combined with path")
    func testUrlConstruction() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.absoluteString.hasPrefix("http://localhost:8080/api/v1/queries/42") == true)
            return try mockResponse(json: [
                "inProgress": false,
                "totalPeers": 0,
                "peersQueried": 0,
                "numPeerResponses": 0,
                "totalHits": 0
            ])
        }

        let client = createMockClient()
        let _: QueryStatusResponse = try await client.get("/queries/42")
        await client.shutdown()
    }
}
