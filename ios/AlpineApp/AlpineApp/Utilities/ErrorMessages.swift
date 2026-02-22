import Foundation

enum ErrorMessages {
    static func userFriendly(from error: Error) -> String {
        if let rpcError = error as? JsonRpcError {
            return "Server error (\(rpcError.code)): \(rpcError.message)"
        }
        if let urlError = error as? URLError {
            switch urlError.code {
            case .notConnectedToInternet:
                return "No internet connection"
            case .timedOut:
                return "Connection timed out"
            case .cannotFindHost:
                return "Server not found"
            case .cannotConnectToHost:
                return "Cannot connect to server"
            case .secureConnectionFailed:
                return "TLS/SSL connection failed"
            default:
                return "Network error: \(urlError.localizedDescription)"
            }
        }
        return error.localizedDescription
    }
}
