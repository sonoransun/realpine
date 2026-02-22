import Foundation

struct JsonRpcError: Error, LocalizedError {
    let code: Int
    let message: String

    var errorDescription: String? { message }
}

actor JsonRpcClient {
    private let baseURL: URL
    private let apiKey: String
    private let session: URLSession
    private var nextId: Int64 = 1

    init(baseURL: URL, apiKey: String = "", tlsConfig: TlsConfig = TlsConfig()) {
        self.baseURL = baseURL
        self.apiKey = apiKey

        let config = URLSessionConfiguration.default
        config.timeoutIntervalForRequest = 30
        config.timeoutIntervalForResource = 30

        self.session = URLSession(configuration: config)
    }

    func call<T: Decodable>(method: String, params: [String: Any] = [:]) async throws -> T {
        let responseData = try await sendRequest(method: method, params: params)

        guard let json = try JSONSerialization.jsonObject(with: responseData) as? [String: Any] else {
            throw JsonRpcError(code: -32600, message: "Invalid JSON response")
        }

        // Check for error
        if let error = json["error"] as? [String: Any] {
            let code = error["code"] as? Int ?? -1
            let message = error["message"] as? String ?? "Unknown error"
            throw JsonRpcError(code: code, message: message)
        }

        // Extract result
        guard let result = json["result"] else {
            throw JsonRpcError(code: -32600, message: "Missing result in response")
        }

        let resultData = try JSONSerialization.data(withJSONObject: result)
        let decoder = JSONDecoder()
        return try decoder.decode(T.self, from: resultData)
    }

    func callVoid(method: String, params: [String: Any] = [:]) async throws {
        let responseData = try await sendRequest(method: method, params: params)

        guard let json = try JSONSerialization.jsonObject(with: responseData) as? [String: Any] else {
            throw JsonRpcError(code: -32600, message: "Invalid JSON response")
        }

        if let error = json["error"] as? [String: Any] {
            let code = error["code"] as? Int ?? -1
            let message = error["message"] as? String ?? "Unknown error"
            throw JsonRpcError(code: code, message: message)
        }
    }

    func shutdown() {
        session.invalidateAndCancel()
    }

    // MARK: - Private

    private func sendRequest(method: String, params: [String: Any]) async throws -> Data {
        let id = nextId
        nextId += 1

        var body: [String: Any] = [
            "jsonrpc": "2.0",
            "method": method,
            "id": id
        ]

        if !params.isEmpty {
            body["params"] = params
        }

        let bodyData = try JSONSerialization.data(withJSONObject: body)

        var request = URLRequest(url: baseURL)
        request.httpMethod = "POST"
        request.httpBody = bodyData
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")

        if !apiKey.isEmpty {
            request.setValue("Bearer \(apiKey)", forHTTPHeaderField: "Authorization")
        }

        let (data, response) = try await session.data(for: request)

        guard let httpResponse = response as? HTTPURLResponse else {
            throw JsonRpcError(code: -32600, message: "Invalid HTTP response")
        }

        guard httpResponse.statusCode == 200 else {
            throw JsonRpcError(
                code: httpResponse.statusCode,
                message: "HTTP error \(httpResponse.statusCode)"
            )
        }

        return data
    }
}
