import Foundation

/// Indexes files in a shared directory and provides search capability
/// using glob-like patterns. Used by BroadcastResponder to answer
/// incoming query broadcasts with matching local files.
final class LocalContentProvider {
    private var indexedFiles: [(relativePath: String, size: Int64)] = []
    private var rootDirectory: String = ""
    private let maxIndexedFiles = 500
    private let maxQueryLength = 256
    private let maxResults = 500

    var fileCount: Int { indexedFiles.count }

    /// Recursively enumerates files in the given directory and builds an
    /// index of relative paths and file sizes, up to maxIndexedFiles.
    func indexDirectory(path: String) {
        rootDirectory = path
        indexedFiles = []

        let fileManager = FileManager.default
        let rootURL = URL(fileURLWithPath: path, isDirectory: true)

        guard let enumerator = fileManager.enumerator(
            at: rootURL,
            includingPropertiesForKeys: [.fileSizeKey, .isRegularFileKey],
            options: [.skipsHiddenFiles, .skipsPackageDescendants]
        ) else {
            return
        }

        var count = 0
        while let fileURL = enumerator.nextObject() as? URL {
            guard count < maxIndexedFiles else { break }

            do {
                let resourceValues = try fileURL.resourceValues(forKeys: [.isRegularFileKey, .fileSizeKey])
                guard resourceValues.isRegularFile == true else { continue }

                let relativePath = fileURL.path.replacingOccurrences(
                    of: rootURL.path,
                    with: ""
                )
                // Strip leading slash from relative path
                let cleanPath = relativePath.hasPrefix("/")
                    ? String(relativePath.dropFirst())
                    : relativePath

                let size = Int64(resourceValues.fileSize ?? 0)
                indexedFiles.append((relativePath: cleanPath, size: size))
                count += 1
            } catch {
                continue
            }
        }
    }

    /// Searches indexed files using a glob-like pattern and returns matching
    /// ResourceDesc entries with HTTP locators pointing to the local file server.
    ///
    /// - Parameters:
    ///   - queryString: Glob pattern (supports * and ? wildcards)
    ///   - localAddress: The local IP address for building file URLs
    ///   - fileServerPort: The port the local file server is listening on
    /// - Returns: Array of ResourceDesc for matching files
    func search(queryString: String, localAddress: String, fileServerPort: Int) -> [ResourceDesc] {
        let trimmed = String(queryString.prefix(maxQueryLength))
            .trimmingCharacters(in: .whitespacesAndNewlines)

        guard !trimmed.isEmpty else { return [] }

        let regexPattern = globToRegex(trimmed)

        guard let regex = try? NSRegularExpression(
            pattern: regexPattern,
            options: .caseInsensitive
        ) else {
            return []
        }

        var results: [ResourceDesc] = []
        var resourceId: Int64 = 1

        for file in indexedFiles {
            guard results.count < maxResults else { break }

            let range = NSRange(file.relativePath.startIndex..., in: file.relativePath)
            if regex.firstMatch(in: file.relativePath, range: range) != nil {
                let encodedPath = file.relativePath.addingPercentEncoding(
                    withAllowedCharacters: .urlPathAllowed
                ) ?? file.relativePath

                let locator = "http://\(localAddress):\(fileServerPort)/files/\(encodedPath)"

                let desc = ResourceDesc(
                    resourceId: resourceId,
                    size: file.size,
                    locators: [locator],
                    description: file.relativePath
                )
                results.append(desc)
                resourceId += 1
            }
        }

        return results
    }

    /// Converts a glob-like pattern to a regular expression.
    /// Supports * (match any sequence) and ? (match single character).
    private func globToRegex(_ glob: String) -> String {
        var regex = "^"
        for char in glob {
            switch char {
            case "*":
                regex += ".*"
            case "?":
                regex += "."
            case ".":
                regex += "\\."
            case "(", ")", "[", "]", "{", "}", "+", "^", "$", "|", "\\":
                regex += "\\\(char)"
            default:
                regex += String(char)
            }
        }
        regex += "$"
        return regex
    }
}
