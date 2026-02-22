import Testing
import Foundation
@testable import AlpineApp

@Suite("ApiError Tests")
struct ApiErrorTests {

    // MARK: - Error Description

    @Test("httpError provides descriptive message with status code")
    func testHttpErrorDescription() {
        let error = ApiError.httpError(statusCode: 404, message: "Not found")
        let description = error.errorDescription ?? ""
        #expect(description.contains("404"))
        #expect(description.contains("Not found"))
    }

    @Test("httpError with 500 status code")
    func testHttpError500() {
        let error = ApiError.httpError(statusCode: 500, message: "Internal server error")
        let description = error.errorDescription ?? ""
        #expect(description.contains("500"))
        #expect(description.contains("Internal server error"))
    }

    @Test("httpError with empty message")
    func testHttpErrorEmptyMessage() {
        let error = ApiError.httpError(statusCode: 503, message: "")
        let description = error.errorDescription ?? ""
        #expect(description.contains("503"))
    }

    @Test("invalidResponse provides message")
    func testInvalidResponse() {
        let error = ApiError.invalidResponse
        let description = error.errorDescription ?? ""
        #expect(description.contains("Invalid") || description.contains("invalid"))
    }

    @Test("encodingFailed provides message")
    func testEncodingFailed() {
        let error = ApiError.encodingFailed
        let description = error.errorDescription ?? ""
        #expect(description.contains("encode") || description.contains("Encode"))
    }

    @Test("decodingFailed wraps underlying error")
    func testDecodingFailed() {
        let underlying = DecodingError.dataCorrupted(
            .init(codingPath: [], debugDescription: "bad data")
        )
        let error = ApiError.decodingFailed(underlying: underlying)
        let description = error.errorDescription ?? ""
        #expect(description.contains("decode") || description.contains("Decode"))
    }

    @Test("noData provides message")
    func testNoData() {
        let error = ApiError.noData
        let description = error.errorDescription ?? ""
        #expect(description.contains("Empty") || description.contains("empty") || description.contains("response"))
    }

    @Test("networkError wraps underlying URLError")
    func testNetworkErrorTimeout() {
        let underlying = URLError(.timedOut)
        let error = ApiError.networkError(underlying: underlying)
        let description = error.errorDescription ?? ""
        #expect(!description.isEmpty)
    }

    @Test("networkError wraps connection refused")
    func testNetworkErrorConnectionRefused() {
        let underlying = URLError(.cannotConnectToHost)
        let error = ApiError.networkError(underlying: underlying)
        let description = error.errorDescription ?? ""
        #expect(!description.isEmpty)
    }

    @Test("networkError wraps not connected to internet")
    func testNetworkErrorNotConnected() {
        let underlying = URLError(.notConnectedToInternet)
        let error = ApiError.networkError(underlying: underlying)
        let description = error.errorDescription ?? ""
        #expect(!description.isEmpty)
    }

    @Test("invalidURL includes the URL string")
    func testInvalidURL() {
        let error = ApiError.invalidURL("bad://url")
        let description = error.errorDescription ?? ""
        #expect(description.contains("bad://url"))
    }

    @Test("invalidURL with empty string")
    func testInvalidURLEmpty() {
        let error = ApiError.invalidURL("")
        let description = error.errorDescription ?? ""
        #expect(description.contains("URL") || description.contains("url"))
    }

    // MARK: - Error Protocol Conformance

    @Test("ApiError conforms to Error protocol")
    func testErrorConformance() {
        let error: Error = ApiError.invalidResponse
        #expect(error is ApiError)
    }

    @Test("ApiError conforms to LocalizedError protocol")
    func testLocalizedErrorConformance() {
        let error: LocalizedError = ApiError.invalidResponse
        #expect(error.errorDescription != nil)
    }

    @Test("ApiError conforms to Sendable")
    func testSendableConformance() {
        // Verify Sendable conformance by passing across concurrency boundary
        let error: any Sendable = ApiError.httpError(statusCode: 404, message: "test")
        #expect(error is ApiError)
    }

    // MARK: - localizedDescription (via NSError bridge)

    @Test("localizedDescription works via NSError bridge")
    func testLocalizedDescription() {
        let error = ApiError.httpError(statusCode: 404, message: "Not found")
        // localizedDescription should use errorDescription from LocalizedError
        #expect(error.localizedDescription.contains("404"))
    }

    // MARK: - Pattern Matching

    @Test("httpError can be pattern-matched for status code")
    func testHttpErrorPatternMatch() {
        let error = ApiError.httpError(statusCode: 401, message: "Unauthorized")
        if case .httpError(let code, let msg) = error {
            #expect(code == 401)
            #expect(msg == "Unauthorized")
        } else {
            Issue.record("Pattern match failed")
        }
    }

    @Test("decodingFailed can extract underlying error")
    func testDecodingFailedPatternMatch() {
        let underlying = DecodingError.keyNotFound(
            CodingKeys.test,
            .init(codingPath: [], debugDescription: "missing key")
        )
        let error = ApiError.decodingFailed(underlying: underlying)
        if case .decodingFailed(let inner) = error {
            #expect(inner is DecodingError)
        } else {
            Issue.record("Pattern match failed")
        }
    }

    @Test("networkError can extract underlying error")
    func testNetworkErrorPatternMatch() {
        let underlying = URLError(.timedOut)
        let error = ApiError.networkError(underlying: underlying)
        if case .networkError(let inner) = error {
            #expect(inner is URLError)
        } else {
            Issue.record("Pattern match failed")
        }
    }
}

// Helper enum for testing DecodingError with CodingKey
private enum CodingKeys: String, CodingKey {
    case test
}
