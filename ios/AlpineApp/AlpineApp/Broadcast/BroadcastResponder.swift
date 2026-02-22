import Foundation
import Network
import os

private let logger = Logger(subsystem: "com.sonoranpub.AlpineApp", category: "BroadcastResponder")

/// Listens on UDP port 8090 for incoming Alpine query broadcasts from peers
/// on the local network. When a query is received, searches local content
/// and sends back a response with matching file resources.
final class BroadcastResponder {
    private var listener: NWListener?
    private let port: UInt16 = 8090
    private let localContentProvider: LocalContentProvider
    private let fileServerPort: Int
    private let queue = DispatchQueue(label: "com.alpine.broadcast.responder", qos: .utility)

    /// Rate limiter: tracks query timestamps per senderId.
    private var rateLimiter: [String: [Date]] = [:]
    private let rateLock = NSLock()
    private let maxQueriesPerWindow = 5
    private let rateLimitWindow: TimeInterval = 10.0

    /// A unique responder ID for this device.
    private let responderId: String

    init(localContentProvider: LocalContentProvider, fileServerPort: Int) {
        self.localContentProvider = localContentProvider
        self.fileServerPort = fileServerPort
        self.responderId = String(UUID().uuidString.prefix(8)).lowercased()
    }

    /// Starts listening for query broadcasts on UDP port 8090.
    func start() {
        do {
            let params = NWParameters.udp
            params.allowLocalEndpointReuse = true
            params.requiredInterfaceType = .wifi

            listener = try NWListener(using: params, on: NWEndpoint.Port(integerLiteral: port))
        } catch {
            logger.error("Failed to create listener: \(error.localizedDescription)")
            return
        }

        listener?.newConnectionHandler = { [weak self] connection in
            self?.handleConnection(connection)
        }

        let listenPort = port
        listener?.stateUpdateHandler = { state in
            switch state {
            case .ready:
                logger.info("Listening for queries on port \(listenPort)")
            case .failed(let error):
                logger.error("Listener failed: \(error.localizedDescription)")
            default:
                break
            }
        }

        listener?.start(queue: queue)
    }

    /// Stops the broadcast responder and cancels the listener.
    func stop() {
        listener?.cancel()
        listener = nil

        rateLock.lock()
        rateLimiter.removeAll()
        rateLock.unlock()
    }

    // MARK: - Connection Handling

    private func handleConnection(_ connection: NWConnection) {
        connection.start(queue: queue)

        connection.receiveMessage { [weak self] data, _, _, error in
            defer { connection.cancel() }

            if let error {
                logger.error("Receive error: \(error.localizedDescription)")
                return
            }

            guard let data else { return }

            self?.handleQuery(data: data, from: connection.currentPath?.remoteEndpoint)
        }
    }

    // MARK: - Query Handling

    private func handleQuery(data: Data, from endpoint: NWEndpoint?) {
        guard let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any] else {
            return
        }

        // Only process query messages
        guard let type = json["type"] as? String, type == "alpine_query" else {
            return
        }

        guard let queryId = json["queryId"] as? Int64,
              let senderId = json["senderId"] as? String,
              let queryString = json["queryString"] as? String else {
            return
        }

        // Do not respond to our own broadcasts
        if senderId == responderId {
            return
        }

        // Apply rate limiting per sender
        if isRateLimited(senderId: senderId) {
            logger.warning("Rate limited sender: \(senderId)")
            return
        }

        // Parse optional enhanced search fields
        let searchModeRaw = json["searchMode"] as? String
        let searchMode = searchModeRaw.flatMap { SearchMode(rawValue: $0) }
        let metricRaw = json["similarityMetric"] as? String
        let similarityMetric = metricRaw.flatMap { SimilarityMetric(rawValue: $0) } ?? .cosine
        let similarityThreshold = json["similarityThreshold"] as? Float ?? 0.3
        let protocolVersion = json["protocolVersion"] as? Int ?? 1

        var entityFilters: [EntityFilter]?
        if let filtersJson = json["entityFilters"] as? [[String: Any]] {
            entityFilters = filtersJson.compactMap { dict in
                guard let text = dict["text"] as? String else { return nil }
                let typeRaw = dict["entityType"] as? String
                let entityType = typeRaw.flatMap { EntityType(rawValue: $0) }
                let minConfidence = dict["minConfidence"] as? Double ?? 0.0
                return EntityFilter(text: text, entityType: entityType, minConfidence: minConfidence)
            }
        }

        var sparqlPatterns: [CodableTriplePattern]?
        if let patternsJson = json["sparqlPatterns"] as? [[String: Any]] {
            sparqlPatterns = patternsJson.map { dict in
                CodableTriplePattern(
                    subject: dict["subject"] as? String,
                    predicate: dict["predicate"] as? String,
                    object: dict["object"] as? String
                )
            }
        }

        // Parse v3 fields
        let contentCategoriesRaw = json["contentCategories"] as? [String]
        let contentCategories = contentCategoriesRaw?.compactMap { ContentCategory(rawValue: $0) }
        let languages = json["languages"] as? [String]
        var signalWeights: SignalWeights?
        if let weightsDict = json["signalWeights"] as? [String: Any] {
            signalWeights = SignalWeights(
                text: weightsDict["text"] as? Double ?? 1.0,
                entity: weightsDict["entity"] as? Double ?? 1.0,
                category: weightsDict["category"] as? Double ?? 0.8,
                language: weightsDict["language"] as? Double ?? 0.5
            )
        }

        // Search local content using enhanced or basic search
        let localAddress = NetworkUtility.localIPAddress() ?? "0.0.0.0"
        let results: [ResourceDesc]
        if let searchMode {
            results = localContentProvider.enhancedSearch(
                queryString: queryString,
                localAddress: localAddress,
                fileServerPort: fileServerPort,
                searchMode: searchMode,
                similarityMetric: similarityMetric,
                similarityThreshold: similarityThreshold,
                entityFilters: entityFilters,
                sparqlPatterns: sparqlPatterns,
                contentCategories: contentCategories,
                languages: languages,
                signalWeights: signalWeights
            )
        } else {
            results = localContentProvider.search(
                queryString: queryString,
                localAddress: localAddress,
                fileServerPort: fileServerPort
            )
        }

        // Only send a response if we have matching results
        guard !results.isEmpty else { return }

        // Send the response back to the sender
        if let endpoint {
            sendResponse(queryId: queryId, resources: results, to: endpoint, protocolVersion: protocolVersion)
        } else if let senderAddress = json["senderAddress"] as? String,
                  let senderPort = json["senderPort"] as? Int {
            // Fall back to the address/port specified in the query message
            let target = NWEndpoint.hostPort(
                host: NWEndpoint.Host(senderAddress),
                port: NWEndpoint.Port(integerLiteral: UInt16(senderPort))
            )
            sendResponse(queryId: queryId, resources: results, to: target, protocolVersion: protocolVersion)
        }
    }

    // MARK: - Rate Limiting

    /// Checks whether the given senderId has exceeded the rate limit.
    /// Also records the current timestamp for the sender.
    private func isRateLimited(senderId: String) -> Bool {
        let now = Date()

        rateLock.lock()
        defer { rateLock.unlock() }

        // Clean up old entries outside the rate limit window
        var timestamps = rateLimiter[senderId] ?? []
        timestamps.removeAll { now.timeIntervalSince($0) > rateLimitWindow }

        if timestamps.count >= maxQueriesPerWindow {
            return true
        }

        timestamps.append(now)
        rateLimiter[senderId] = timestamps
        return false
    }

    // MARK: - Response Sending

    private func sendResponse(queryId: Int64, resources: [ResourceDesc], to endpoint: NWEndpoint, protocolVersion: Int = 1) {
        // Build the resource array for JSON serialization
        var resourceDicts: [[String: Any]] = resources.map { resource in
            var dict: [String: Any] = [
                "resourceId": resource.resourceId,
                "size": resource.size,
                "description": resource.description,
                "locators": resource.locators
            ]

            // Include enhanced fields for protocol version >= 2
            if protocolVersion >= 2 {
                if let score = resource.score {
                    dict["score"] = score
                }
                if let matchedKeywords = resource.matchedKeywords {
                    dict["matchedKeywords"] = matchedKeywords
                }
                if let matchedEntities = resource.matchedEntities {
                    dict["matchedEntities"] = matchedEntities.map { entity in
                        [
                            "text": entity.text,
                            "entityType": entity.entityType.rawValue,
                            "confidence": entity.confidence,
                            "source": entity.source.rawValue
                        ] as [String: Any]
                    }
                }
                if let rdfTriples = resource.rdfTriples {
                    dict["rdfTriples"] = rdfTriples.map { triple in
                        var tripleDict: [String: Any] = ["predicate": triple.predicate]
                        switch triple.subject {
                        case .uri(let uri):
                            tripleDict["subject"] = uri
                        case .literal(let value, _):
                            tripleDict["subject"] = value
                        case .blank(let id):
                            tripleDict["subject"] = "_:\(id)"
                        }
                        switch triple.object {
                        case .uri(let uri):
                            tripleDict["object"] = uri
                        case .literal(let value, _):
                            tripleDict["object"] = value
                        case .blank(let id):
                            tripleDict["object"] = "_:\(id)"
                        }
                        return tripleDict
                    }
                }
            }

            // Include v3 fields for protocol version >= 3
            if protocolVersion >= 3 {
                if let contentCategory = resource.contentCategory {
                    dict["contentCategory"] = contentCategory.rawValue
                }
                if let language = resource.language {
                    dict["language"] = [
                        "languageCode": language.languageCode,
                        "confidence": language.confidence
                    ] as [String: Any]
                }
                if let media = resource.mediaMetadata {
                    var mediaDict: [String: Any] = [:]
                    if let w = media.width { mediaDict["width"] = w }
                    if let h = media.height { mediaDict["height"] = h }
                    if let dur = media.durationSeconds { mediaDict["durationSeconds"] = dur }
                    if let cs = media.colorSpace { mediaDict["colorSpace"] = cs }
                    if let alpha = media.hasAlpha { mediaDict["hasAlpha"] = alpha }
                    if let codec = media.codec { mediaDict["codec"] = codec }
                    if let br = media.bitRate { mediaDict["bitRate"] = br }
                    if let sr = media.sampleRate { mediaDict["sampleRate"] = sr }
                    if !mediaDict.isEmpty {
                        dict["mediaMetadata"] = mediaDict
                    }
                }
                if let refinements = resource.refinements {
                    dict["refinements"] = refinements.map { ref in
                        var refDict: [String: Any] = [
                            "id": ref.id.uuidString,
                            "label": ref.label,
                            "refinementType": ref.refinementType.rawValue
                        ]
                        var params: [String: Any] = [:]
                        if let cat = ref.parameters.contentCategory { params["contentCategory"] = cat.rawValue }
                        if let lang = ref.parameters.languageCode { params["languageCode"] = lang }
                        if let et = ref.parameters.entityType { params["entityType"] = et.rawValue }
                        if let txt = ref.parameters.entityText { params["entityText"] = txt }
                        if let kw = ref.parameters.keyword { params["keyword"] = kw }
                        if let t = ref.parameters.threshold { params["threshold"] = t }
                        refDict["parameters"] = params
                        return refDict
                    }
                }
            }

            return dict
        }

        let responseMessage: [String: Any] = [
            "type": "alpine_response",
            "queryId": queryId,
            "responderId": responderId,
            "resources": resourceDicts
        ]

        guard var data = try? JSONSerialization.data(withJSONObject: responseMessage) else {
            logger.error("Failed to serialize response")
            return
        }

        // UDP size guard: truncate resources if serialized data exceeds 60KB
        let maxUDPSize = 60000
        if data.count > maxUDPSize {
            // Binary search for the maximum number of resources that fits
            var low = 0
            var high = resourceDicts.count
            var bestCount = 0

            while low <= high {
                let mid = (low + high) / 2
                let truncatedMessage: [String: Any] = [
                    "type": "alpine_response",
                    "queryId": queryId,
                    "responderId": responderId,
                    "resources": Array(resourceDicts.prefix(mid))
                ]
                if let testData = try? JSONSerialization.data(withJSONObject: truncatedMessage),
                   testData.count <= maxUDPSize {
                    bestCount = mid
                    low = mid + 1
                } else {
                    high = mid - 1
                }
            }

            resourceDicts = Array(resourceDicts.prefix(bestCount))
            let truncatedMessage: [String: Any] = [
                "type": "alpine_response",
                "queryId": queryId,
                "responderId": responderId,
                "resources": resourceDicts
            ]
            guard let truncatedData = try? JSONSerialization.data(withJSONObject: truncatedMessage) else {
                logger.error("Failed to serialize truncated response")
                return
            }
            data = truncatedData
        }

        let params = NWParameters.udp
        params.allowLocalEndpointReuse = true

        let connection = NWConnection(to: endpoint, using: params)
        connection.start(queue: queue)

        connection.send(content: data, completion: .contentProcessed { error in
            if let error {
                logger.error("Send response error: \(error.localizedDescription)")
            }
            // Brief delay before cancelling to ensure delivery
            DispatchQueue.global().asyncAfter(deadline: .now() + 0.3) {
                connection.cancel()
            }
        })
    }
}
