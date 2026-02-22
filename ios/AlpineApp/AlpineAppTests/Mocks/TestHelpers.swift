import Foundation
@testable import AlpineApp

/// Creates a RestApiClient configured with a localhost base URL.
/// Use alongside MockURLProtocol.registerClass to intercept requests.
func createMockClient(baseURL: String = "http://localhost:8080/api/v1") -> RestApiClient {
    RestApiClient(baseURL: URL(string: baseURL)!)
}

/// Creates an AlpineApiService backed by a mock-friendly client.
func createMockApiService(baseURL: String = "http://localhost:8080/api/v1") -> AlpineApiService {
    AlpineApiService(client: createMockClient(baseURL: baseURL))
}

/// Helper to create a mock HTTP response from a JSON-serializable dictionary or array.
func mockResponse(
    url: String = "http://localhost:8080/api/v1",
    statusCode: Int = 200,
    json: Any
) throws -> (HTTPURLResponse, Data) {
    let data = try JSONSerialization.data(withJSONObject: json)
    let response = HTTPURLResponse(
        url: URL(string: url)!,
        statusCode: statusCode,
        httpVersion: "HTTP/1.1",
        headerFields: ["Content-Type": "application/json"]
    )!
    return (response, data)
}

/// Helper to create a mock HTTP response from a Codable object.
func mockResponseCodable<T: Encodable>(
    url: String = "http://localhost:8080/api/v1",
    statusCode: Int = 200,
    body: T
) throws -> (HTTPURLResponse, Data) {
    let data = try JSONEncoder().encode(body)
    let response = HTTPURLResponse(
        url: URL(string: url)!,
        statusCode: statusCode,
        httpVersion: "HTTP/1.1",
        headerFields: ["Content-Type": "application/json"]
    )!
    return (response, data)
}
