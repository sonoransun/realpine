import Foundation

/// Represents errors from the Alpine REST API.
enum ApiError: Error, LocalizedError, Sendable {
    /// HTTP error with status code and optional body message
    case httpError(statusCode: Int, message: String)
    /// Server returned invalid or unexpected response
    case invalidResponse
    /// Failed to encode request body
    case encodingFailed
    /// Failed to decode response body
    case decodingFailed(underlying: Error)
    /// No data in response when data was expected
    case noData
    /// Network connectivity issue
    case networkError(underlying: Error)
    /// Invalid URL configuration
    case invalidURL(String)
    /// Authentication is required but no credentials provided
    case authenticationRequired
    /// Session has expired and needs re-authentication
    case sessionExpired
    /// No network connection available
    case networkUnavailable
    /// Request failed after multiple retries
    case retryExhausted(lastError: String)
    /// Request timed out
    case timeout

    var errorDescription: String? {
        switch self {
        case .httpError(let code, let message):
            "Server error (\(code)): \(message)"
        case .invalidResponse:
            "Invalid server response"
        case .encodingFailed:
            "Failed to encode request"
        case .decodingFailed(let error):
            "Failed to decode response: \(error.localizedDescription)"
        case .noData:
            "Empty response from server"
        case .networkError(let error):
            error.localizedDescription
        case .invalidURL(let url):
            "Invalid URL: \(url)"
        case .authenticationRequired:
            "Authentication required"
        case .sessionExpired:
            "Session expired, please re-authenticate"
        case .networkUnavailable:
            "No network connection available"
        case .retryExhausted(let lastError):
            "Request failed after multiple retries: \(lastError)"
        case .timeout:
            "Request timed out"
        }
    }
}
