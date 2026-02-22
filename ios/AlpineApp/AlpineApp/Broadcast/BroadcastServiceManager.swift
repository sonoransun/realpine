import Foundation
import Observation

/// Orchestrates the broadcast services: LocalContentProvider for file indexing,
/// LocalFileServer for serving files over HTTP, and BroadcastResponder for
/// answering query broadcasts from peers on the local network.
@Observable
final class BroadcastServiceManager {
    private(set) var isActive = false
    private(set) var indexedFileCount = 0

    private var responder: BroadcastResponder?
    private var fileServer: LocalFileServer?
    private let contentProvider: LocalContentProvider

    init() {
        contentProvider = LocalContentProvider()
    }

    /// Starts all broadcast services: indexes the shared directory, starts the
    /// file server, and begins listening for query broadcasts.
    ///
    /// - Parameters:
    ///   - sharedDirectory: Path to the directory containing files to share
    ///   - fileServerPort: Port for the HTTP file server (default 8091)
    func start(sharedDirectory: String, fileServerPort: Int) {
        guard !isActive else { return }

        // Index the shared directory
        contentProvider.indexDirectory(path: sharedDirectory)
        indexedFileCount = contentProvider.fileCount

        // Start the local file server
        fileServer = LocalFileServer(rootDirectory: sharedDirectory, port: UInt16(fileServerPort))
        fileServer?.start()

        // Start the broadcast responder
        responder = BroadcastResponder(localContentProvider: contentProvider, fileServerPort: fileServerPort)
        responder?.start()

        isActive = true
    }

    /// Stops all broadcast services: the responder, file server, and clears state.
    func stop() {
        responder?.stop()
        responder = nil

        fileServer?.stop()
        fileServer = nil

        isActive = false
    }

    /// Re-indexes the shared directory, updating the file count.
    /// Call this when files in the shared directory have changed.
    ///
    /// - Parameter sharedDirectory: Path to the directory containing files to share
    func reindex(sharedDirectory: String) {
        contentProvider.indexDirectory(path: sharedDirectory)
        indexedFileCount = contentProvider.fileCount
    }
}
