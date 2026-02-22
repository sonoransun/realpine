import Foundation

/// Manages server session token lifecycle: authentication, refresh, invalidation.
actor SessionManager {
    private let session: URLSession
    private let encoder: JSONEncoder
    private let decoder: JSONDecoder

    init() {
        let config = URLSessionConfiguration.default
        config.timeoutIntervalForRequest = 30
        self.session = URLSession(configuration: config)
        self.encoder = JSONEncoder()
        self.decoder = JSONDecoder()
    }

    /// Authenticate with the server and obtain a session token.
    func authenticate(request: AuthRequest, baseURL: URL) async throws -> SessionToken {
        let url = baseURL.appendingPathComponent("auth/login")
        var urlRequest = URLRequest(url: url)
        urlRequest.httpMethod = "POST"
        urlRequest.setValue("application/json", forHTTPHeaderField: "Content-Type")
        urlRequest.httpBody = try encoder.encode(request)

        let (data, response) = try await session.data(for: urlRequest)
        try validateResponse(response)

        let authResponse = try decoder.decode(AuthResponse.self, from: data)
        let now = Date()
        return SessionToken(
            accessToken: authResponse.accessToken,
            refreshToken: authResponse.refreshToken,
            expiresAt: now.addingTimeInterval(Double(authResponse.expiresIn)),
            issuedAt: now
        )
    }

    /// Refresh an existing session using the refresh token.
    func refreshSession(refreshToken: String, baseURL: URL) async throws -> SessionToken {
        let url = baseURL.appendingPathComponent("auth/refresh")
        var urlRequest = URLRequest(url: url)
        urlRequest.httpMethod = "POST"
        urlRequest.setValue("application/json", forHTTPHeaderField: "Content-Type")
        urlRequest.setValue("Bearer \(refreshToken)", forHTTPHeaderField: "Authorization")

        let (data, response) = try await session.data(for: urlRequest)
        try validateResponse(response)

        let authResponse = try decoder.decode(AuthResponse.self, from: data)
        let now = Date()
        return SessionToken(
            accessToken: authResponse.accessToken,
            refreshToken: authResponse.refreshToken,
            expiresAt: now.addingTimeInterval(Double(authResponse.expiresIn)),
            issuedAt: now
        )
    }

    /// Invalidate the current session (logout).
    func invalidateSession(accessToken: String, baseURL: URL) async throws {
        let url = baseURL.appendingPathComponent("auth/logout")
        var urlRequest = URLRequest(url: url)
        urlRequest.httpMethod = "POST"
        urlRequest.setValue("Bearer \(accessToken)", forHTTPHeaderField: "Authorization")

        let (_, response) = try await session.data(for: urlRequest)
        try validateResponse(response)
    }

    /// Request a challenge from the server for YubiKey authentication.
    func requestChallenge(baseURL: URL) async throws -> AuthChallenge {
        let url = baseURL.appendingPathComponent("auth/challenge")
        var urlRequest = URLRequest(url: url)
        urlRequest.httpMethod = "GET"
        urlRequest.setValue("application/json", forHTTPHeaderField: "Accept")

        let (data, response) = try await session.data(for: urlRequest)
        try validateResponse(response)

        return try decoder.decode(AuthChallenge.self, from: data)
    }

    private func validateResponse(_ response: URLResponse) throws {
        guard let httpResponse = response as? HTTPURLResponse else {
            throw ApiError.invalidResponse
        }
        guard (200...299).contains(httpResponse.statusCode) else {
            if httpResponse.statusCode == 401 {
                throw ApiError.sessionExpired
            }
            throw ApiError.httpError(statusCode: httpResponse.statusCode, message: "Authentication failed")
        }
    }
}
