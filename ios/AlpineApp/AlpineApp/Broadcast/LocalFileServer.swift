import Foundation
import Network

/// A simple HTTP file server using the Network framework.
/// Serves files from a root directory over TCP, allowing peers on the
/// local network to download shared files via HTTP GET requests.
final class LocalFileServer {
    private var listener: NWListener?
    private let rootDirectory: String
    private let port: UInt16
    private let queue = DispatchQueue(label: "com.alpine.fileserver", qos: .utility)

    init(rootDirectory: String, port: UInt16 = 8091) {
        self.rootDirectory = rootDirectory
        self.port = port
    }

    /// Starts the HTTP file server on the configured TCP port.
    func start() {
        do {
            let params = NWParameters.tcp
            listener = try NWListener(using: params, on: NWEndpoint.Port(integerLiteral: port))
        } catch {
            print("[LocalFileServer] Failed to create listener: \(error)")
            return
        }

        listener?.newConnectionHandler = { [weak self] connection in
            self?.handleConnection(connection)
        }

        let listenPort = port
        listener?.stateUpdateHandler = { state in
            switch state {
            case .ready:
                print("[LocalFileServer] Listening on port \(listenPort)")
            case .failed(let error):
                print("[LocalFileServer] Listener failed: \(error)")
            default:
                break
            }
        }

        listener?.start(queue: queue)
    }

    /// Stops the file server and cancels the listener.
    func stop() {
        listener?.cancel()
        listener = nil
    }

    // MARK: - Connection Handling

    private func handleConnection(_ connection: NWConnection) {
        connection.start(queue: queue)

        // Read the HTTP request (up to 8KB should be sufficient for a GET request)
        connection.receive(minimumIncompleteLength: 1, maximumLength: 8192) { [weak self] data, _, _, error in
            guard let self else {
                connection.cancel()
                return
            }

            if let error {
                print("[LocalFileServer] Receive error: \(error)")
                connection.cancel()
                return
            }

            guard let data, let requestString = String(data: data, encoding: .utf8) else {
                self.sendErrorResponse(connection: connection, statusCode: 400, message: "Bad Request")
                return
            }

            self.processRequest(requestString, connection: connection)
        }
    }

    private func processRequest(_ requestString: String, connection: NWConnection) {
        // Parse the HTTP request line: "GET /files/path HTTP/1.1"
        let lines = requestString.components(separatedBy: "\r\n")
        guard let requestLine = lines.first else {
            sendErrorResponse(connection: connection, statusCode: 400, message: "Bad Request")
            return
        }

        let parts = requestLine.components(separatedBy: " ")
        guard parts.count >= 2, parts[0] == "GET" else {
            sendErrorResponse(connection: connection, statusCode: 405, message: "Method Not Allowed")
            return
        }

        var requestPath = parts[1]

        // URL-decode the path
        if let decoded = requestPath.removingPercentEncoding {
            requestPath = decoded
        }

        // Strip the /files/ prefix
        let prefix = "/files/"
        guard requestPath.hasPrefix(prefix) else {
            sendErrorResponse(connection: connection, statusCode: 404, message: "Not Found")
            return
        }

        let relativePath = String(requestPath.dropFirst(prefix.count))

        // Sanitize: reject path traversal attempts
        guard !relativePath.contains("..") && !relativePath.hasPrefix("/") else {
            sendErrorResponse(connection: connection, statusCode: 403, message: "Forbidden")
            return
        }

        // Build the full file path
        let rootURL = URL(fileURLWithPath: rootDirectory, isDirectory: true)
        let fileURL = rootURL.appendingPathComponent(relativePath)

        // Additional safety: ensure the resolved path is within the root directory
        let resolvedPath = fileURL.standardizedFileURL.path
        let resolvedRoot = rootURL.standardizedFileURL.path
        guard resolvedPath.hasPrefix(resolvedRoot) else {
            sendErrorResponse(connection: connection, statusCode: 403, message: "Forbidden")
            return
        }

        // Read and serve the file
        let fileManager = FileManager.default
        guard fileManager.fileExists(atPath: resolvedPath),
              let fileData = fileManager.contents(atPath: resolvedPath) else {
            sendErrorResponse(connection: connection, statusCode: 404, message: "Not Found")
            return
        }

        let contentType = mimeType(for: relativePath)
        sendFileResponse(connection: connection, data: fileData, contentType: contentType)
    }

    // MARK: - Response Helpers

    private func sendFileResponse(connection: NWConnection, data: Data, contentType: String) {
        let header = [
            "HTTP/1.1 200 OK",
            "Content-Type: \(contentType)",
            "Content-Length: \(data.count)",
            "Connection: close",
            "",
            ""
        ].joined(separator: "\r\n")

        guard var responseData = header.data(using: .utf8) else {
            connection.cancel()
            return
        }
        responseData.append(data)

        connection.send(content: responseData, completion: .contentProcessed { _ in
            connection.cancel()
        })
    }

    private func sendErrorResponse(connection: NWConnection, statusCode: Int, message: String) {
        let body = "<html><body><h1>\(statusCode) \(message)</h1></body></html>"
        let header = [
            "HTTP/1.1 \(statusCode) \(message)",
            "Content-Type: text/html",
            "Content-Length: \(body.utf8.count)",
            "Connection: close",
            "",
            ""
        ].joined(separator: "\r\n")

        let responseString = header + body
        guard let responseData = responseString.data(using: .utf8) else {
            connection.cancel()
            return
        }

        connection.send(content: responseData, completion: .contentProcessed { _ in
            connection.cancel()
        })
    }

    // MARK: - MIME Type

    private func mimeType(for path: String) -> String {
        let ext = (path as NSString).pathExtension.lowercased()
        switch ext {
        case "html", "htm":
            return "text/html"
        case "txt":
            return "text/plain"
        case "json":
            return "application/json"
        case "xml":
            return "application/xml"
        case "jpg", "jpeg":
            return "image/jpeg"
        case "png":
            return "image/png"
        case "gif":
            return "image/gif"
        case "pdf":
            return "application/pdf"
        case "zip":
            return "application/zip"
        case "mp3":
            return "audio/mpeg"
        case "mp4":
            return "video/mp4"
        case "css":
            return "text/css"
        case "js":
            return "application/javascript"
        default:
            return "application/octet-stream"
        }
    }
}
