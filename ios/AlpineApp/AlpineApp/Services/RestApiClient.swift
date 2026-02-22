import Foundation

/// A generic, actor-isolated REST API client using URLSession.
/// Handles authentication, JSON serialization, and error mapping.
actor RestApiClient {
    let baseURL: URL
    private let apiKey: String
    private var sessionToken: String?
    private let session: URLSession
    private let encoder: JSONEncoder
    private let decoder: JSONDecoder

    /// Initialize with base URL (e.g. "http://host:port/api/v1"), optional API key, session token, and TLS config.
    init(baseURL: URL, apiKey: String = "", sessionToken: String? = nil, tlsConfig: TlsConfig = TlsConfig()) {
        self.baseURL = baseURL
        self.apiKey = apiKey
        self.sessionToken = sessionToken

        let config = URLSessionConfiguration.default
        config.timeoutIntervalForRequest = 30
        config.timeoutIntervalForResource = 60
        self.session = URLSession(configuration: config)

        self.encoder = JSONEncoder()
        self.decoder = JSONDecoder()
    }

    /// Update the session token at runtime (e.g. after authentication or refresh).
    func updateSessionToken(_ token: String?) {
        self.sessionToken = token
    }

    // MARK: - HTTP Methods

    /// GET request returning decoded response.
    func get<T: Decodable>(_ path: String, queryItems: [URLQueryItem] = []) async throws -> T {
        let request = try buildRequest(method: "GET", path: path, queryItems: queryItems)
        return try await execute(request)
    }

    /// GET request with no response body (just validates success).
    func get(_ path: String, queryItems: [URLQueryItem] = []) async throws {
        let request = try buildRequest(method: "GET", path: path, queryItems: queryItems)
        try await executeVoid(request)
    }

    /// POST request with body, returning decoded response.
    func post<B: Encodable, T: Decodable>(_ path: String, body: B) async throws -> T {
        var request = try buildRequest(method: "POST", path: path)
        request.httpBody = try encodeBody(body)
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        return try await execute(request)
    }

    /// POST request with body, no response body expected.
    func post<B: Encodable>(_ path: String, body: B) async throws {
        var request = try buildRequest(method: "POST", path: path)
        request.httpBody = try encodeBody(body)
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        try await executeVoid(request)
    }

    /// POST request with no body, returning decoded response.
    func post<T: Decodable>(_ path: String) async throws -> T {
        let request = try buildRequest(method: "POST", path: path)
        return try await execute(request)
    }

    /// POST request with no body, no response body expected.
    func post(_ path: String) async throws {
        let request = try buildRequest(method: "POST", path: path)
        try await executeVoid(request)
    }

    /// PUT request with body, returning decoded response.
    func put<B: Encodable, T: Decodable>(_ path: String, body: B) async throws -> T {
        var request = try buildRequest(method: "PUT", path: path)
        request.httpBody = try encodeBody(body)
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        return try await execute(request)
    }

    /// PUT request with no body, returning decoded response.
    func put<T: Decodable>(_ path: String) async throws -> T {
        let request = try buildRequest(method: "PUT", path: path)
        return try await execute(request)
    }

    /// PUT request with no body, no response body expected.
    func put(_ path: String) async throws {
        let request = try buildRequest(method: "PUT", path: path)
        try await executeVoid(request)
    }

    /// DELETE request returning decoded response.
    func delete<T: Decodable>(_ path: String) async throws -> T {
        let request = try buildRequest(method: "DELETE", path: path)
        return try await execute(request)
    }

    /// DELETE request with no response body.
    func delete(_ path: String) async throws {
        let request = try buildRequest(method: "DELETE", path: path)
        try await executeVoid(request)
    }

    /// Shuts down the underlying URL session.
    func shutdown() {
        session.invalidateAndCancel()
    }

    // MARK: - Internal

    private func buildRequest(method: String, path: String, queryItems: [URLQueryItem] = []) throws -> URLRequest {
        var components = URLComponents(url: baseURL.appendingPathComponent(path), resolvingAgainstBaseURL: true)
        if !queryItems.isEmpty {
            components?.queryItems = queryItems
        }
        guard let url = components?.url else {
            throw ApiError.invalidURL(path)
        }

        var request = URLRequest(url: url)
        request.httpMethod = method
        request.setValue("application/json", forHTTPHeaderField: "Accept")

        if let token = sessionToken, !token.isEmpty {
            request.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
        } else if !apiKey.isEmpty {
            request.setValue("Bearer \(apiKey)", forHTTPHeaderField: "Authorization")
        }

        return request
    }

    private func encodeBody<B: Encodable>(_ body: B) throws -> Data {
        do {
            return try encoder.encode(body)
        } catch {
            throw ApiError.encodingFailed
        }
    }

    private func execute<T: Decodable>(_ request: URLRequest) async throws -> T {
        let (data, response) = try await performRequest(request)

        do {
            return try decoder.decode(T.self, from: data)
        } catch {
            throw ApiError.decodingFailed(underlying: error)
        }
    }

    private func executeVoid(_ request: URLRequest) async throws {
        let _ = try await performRequest(request)
    }

    private func performRequest(_ request: URLRequest) async throws -> (Data, HTTPURLResponse) {
        let data: Data
        let response: URLResponse

        do {
            (data, response) = try await session.data(for: request)
        } catch {
            throw ApiError.networkError(underlying: error)
        }

        guard let httpResponse = response as? HTTPURLResponse else {
            throw ApiError.invalidResponse
        }

        guard (200...299).contains(httpResponse.statusCode) else {
            if httpResponse.statusCode == 401 {
                throw ApiError.sessionExpired
            }
            let message = String(data: data, encoding: .utf8) ?? "Unknown error"
            throw ApiError.httpError(statusCode: httpResponse.statusCode, message: message)
        }

        return (data, httpResponse)
    }
}
