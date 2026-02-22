import Foundation
import os

private let logger = Logger(subsystem: "com.sonoranpub.AlpineApp", category: "RestApiClient")

/// A generic, actor-isolated REST API client using URLSession.
/// Handles authentication, JSON serialization, and error mapping.
actor RestApiClient {
    let baseURL: URL
    private let apiKey: String
    private var sessionToken: String?
    private let session: URLSession
    private let encoder: JSONEncoder
    private let decoder: JSONDecoder

    /// HTTP status codes eligible for automatic retry.
    private static let retryableStatusCodes: Set<Int> = [408, 429, 500, 502, 503, 504]
    private static let maxRetries = 3

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
        let url = request.url?.absoluteString ?? "unknown"
        let method = request.httpMethod ?? "?"
        logger.debug("Performing \(method) request to \(url)")

        var lastError: Error?

        for attempt in 0..<Self.maxRetries {
            if attempt > 0 {
                // Check for cancellation before retrying
                if Task.isCancelled {
                    logger.warning("Request to \(url) cancelled before retry attempt \(attempt + 1)")
                    throw CancellationError()
                }

                // Exponential backoff: 1s, 2s, 4s with small random jitter
                let baseDelay = Double(1 << attempt)
                let jitter = Double.random(in: 0.0...0.3)
                let delay = baseDelay + jitter
                logger.warning("Retry attempt \(attempt + 1)/\(Self.maxRetries) for \(method) \(url) after \(String(format: "%.1f", delay))s delay")
                try await Task.sleep(for: .seconds(delay))
            }

            let data: Data
            let response: URLResponse

            do {
                (data, response) = try await session.data(for: request)
            } catch {
                lastError = error
                logger.error("Network error on attempt \(attempt + 1) for \(url): \(error.localizedDescription)")
                // Network errors are retryable
                if attempt < Self.maxRetries - 1 {
                    continue
                }
                throw ApiError.networkError(underlying: error)
            }

            guard let httpResponse = response as? HTTPURLResponse else {
                throw ApiError.invalidResponse
            }

            // Non-retryable client errors
            if httpResponse.statusCode == 401 {
                logger.error("Authentication expired for \(url)")
                throw ApiError.sessionExpired
            }

            // Check if this status code is retryable
            if Self.retryableStatusCodes.contains(httpResponse.statusCode) {
                let message = String(data: data, encoding: .utf8) ?? "Unknown error"
                lastError = ApiError.httpError(statusCode: httpResponse.statusCode, message: message)
                logger.warning("Retryable HTTP \(httpResponse.statusCode) on attempt \(attempt + 1) for \(url)")
                if attempt < Self.maxRetries - 1 {
                    continue
                }
                // All retries exhausted
                logger.error("All \(Self.maxRetries) retry attempts exhausted for \(method) \(url)")
                throw ApiError.retryExhausted(lastError: "HTTP \(httpResponse.statusCode): \(message)")
            }

            // Non-retryable HTTP errors
            guard (200...299).contains(httpResponse.statusCode) else {
                let message = String(data: data, encoding: .utf8) ?? "Unknown error"
                logger.error("HTTP \(httpResponse.statusCode) for \(method) \(url): \(message)")
                throw ApiError.httpError(statusCode: httpResponse.statusCode, message: message)
            }

            // Success
            logger.info("Successful \(method) response from \(url) (\(httpResponse.statusCode))")
            return (data, httpResponse)
        }

        // Should not reach here, but safety net
        let errorDesc = lastError?.localizedDescription ?? "Unknown error"
        logger.error("Retry loop exhausted for \(method) \(url): \(errorDesc)")
        throw ApiError.retryExhausted(lastError: errorDesc)
    }
}
