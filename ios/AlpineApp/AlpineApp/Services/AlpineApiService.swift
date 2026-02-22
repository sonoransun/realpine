import Foundation

// MARK: - Request/Response Types

/// Generic success response from the API.
struct SuccessResponse: Codable, Sendable {
    let success: Bool
}

/// Response containing a peer ID.
struct PeerIdResponse: Codable, Sendable {
    let peerId: Int64
}

/// Response containing a group ID.
struct GroupIdResponse: Codable, Sendable {
    let groupId: Int64
}

/// Response containing a module ID.
struct ModuleIdResponse: Codable, Sendable {
    let moduleId: Int64
}

/// Response containing a list of peer IDs.
struct PeerIdsResponse: Codable, Sendable {
    let peerIds: [Int64]
}

/// Response containing a list of group IDs.
struct GroupIdsResponse: Codable, Sendable {
    let groupIds: [Int64]
}

/// Response containing a list of module IDs.
struct ModuleIdsResponse: Codable, Sendable {
    let moduleIds: [Int64]
}

/// Response containing a list of excluded hosts.
struct HostsResponse: Codable, Sendable {
    let hosts: [String]
}

/// Response containing a list of excluded subnets.
struct SubnetsResponse: Codable, Sendable {
    let subnets: [Subnet]
}

/// Request body for adding a peer.
struct AddPeerRequest: Codable, Sendable {
    let ipAddress: String
    let port: Int64
}

/// Request body for creating a group.
struct CreateGroupRequest: Codable, Sendable {
    let name: String
    let description: String
}

/// Request body for registering a module.
struct RegisterModuleRequest: Codable, Sendable {
    let libraryPath: String
    let bootstrapSymbol: String
}

/// Request body for host filter operations.
struct HostFilterRequest: Codable, Sendable {
    let ipAddress: String
}

/// Request body for subnet filter operations.
struct SubnetFilterRequest: Codable, Sendable {
    let subnetIpAddress: String
    let subnetMask: String
}

// MARK: - AlpineApiService

/// REST API service providing typed access to all Alpine server endpoints.
/// Replaces the former JSON-RPC AlpineRpcService with proper RESTful methods.
final class AlpineApiService: Sendable {
    private let client: RestApiClient

    init(client: RestApiClient) {
        self.client = client
    }

    // MARK: - Status

    /// Retrieves the current server status.
    /// `GET /status`
    func getStatus() async throws -> ServerStatus {
        try await client.get("/status")
    }

    // MARK: - Queries

    /// Starts a new query and returns the query ID.
    /// `POST /queries`
    func startQuery(_ request: QueryRequest) async throws -> QueryResponse {
        try await client.post("/queries", body: request)
    }

    /// Starts a new query with individual parameters.
    /// `POST /queries`
    func startQuery(
        queryString: String,
        groupName: String = "",
        autoHaltLimit: Int64 = 100,
        peerDescMax: Int64 = 50
    ) async throws -> Int64 {
        let request = QueryRequest(
            queryString: queryString,
            groupName: groupName,
            autoHaltLimit: autoHaltLimit,
            peerDescMax: peerDescMax
        )
        let response: QueryResponse = try await client.post("/queries", body: request)
        return response.queryId
    }

    /// Gets the status of an active query.
    /// `GET /queries/{queryId}`
    func getQueryStatus(queryId: Int64) async throws -> QueryStatusResponse {
        try await client.get("/queries/\(queryId)")
    }

    /// Gets the results of a query.
    /// `GET /queries/{queryId}/results`
    func getQueryResults(queryId: Int64) async throws -> QueryResultsResponse {
        try await client.get("/queries/\(queryId)/results")
    }

    /// Pauses an active query.
    /// `PUT /queries/{queryId}/pause`
    func pauseQuery(queryId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/queries/\(queryId)/pause")
        return response.success
    }

    /// Resumes a paused query.
    /// `PUT /queries/{queryId}/resume`
    func resumeQuery(queryId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/queries/\(queryId)/resume")
        return response.success
    }

    /// Cancels an active query.
    /// `DELETE /queries/{queryId}`
    func cancelQuery(queryId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.delete("/queries/\(queryId)")
        return response.success
    }

    // MARK: - Peers

    /// Gets all peer IDs.
    /// `GET /peers`
    func getAllPeers() async throws -> [Int64] {
        let response: PeerIdsResponse = try await client.get("/peers")
        return response.peerIds
    }

    /// Gets detailed information about a peer.
    /// `GET /peers/{peerId}`
    func getPeer(peerId: Int64) async throws -> PeerDetail {
        try await client.get("/peers/\(peerId)")
    }

    /// Adds a new peer.
    /// `POST /peers`
    func addPeer(ipAddress: String, port: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.post(
            "/peers",
            body: AddPeerRequest(ipAddress: ipAddress, port: port)
        )
        return response.success
    }

    /// Looks up a peer ID by IP address and port.
    /// `GET /peers/lookup?ip={ip}&port={port}`
    func getPeerId(ipAddress: String, port: Int64) async throws -> Int64 {
        let response: PeerIdResponse = try await client.get(
            "/peers/lookup",
            queryItems: [
                URLQueryItem(name: "ip", value: ipAddress),
                URLQueryItem(name: "port", value: String(port))
            ]
        )
        return response.peerId
    }

    /// Activates a peer.
    /// `PUT /peers/{peerId}/activate`
    func activatePeer(peerId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/peers/\(peerId)/activate")
        return response.success
    }

    /// Deactivates a peer.
    /// `PUT /peers/{peerId}/deactivate`
    func deactivatePeer(peerId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/peers/\(peerId)/deactivate")
        return response.success
    }

    /// Pings a peer to check connectivity.
    /// `POST /peers/{peerId}/ping`
    func pingPeer(peerId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.post("/peers/\(peerId)/ping")
        return response.success
    }

    // MARK: - Filters

    /// Excludes a host from network operations.
    /// `POST /filters/hosts/exclude`
    func excludeHost(ipAddress: String) async throws -> Bool {
        let response: SuccessResponse = try await client.post(
            "/filters/hosts/exclude",
            body: HostFilterRequest(ipAddress: ipAddress)
        )
        return response.success
    }

    /// Excludes a subnet from network operations.
    /// `POST /filters/subnets/exclude`
    func excludeSubnet(subnetIpAddress: String, subnetMask: String) async throws -> Bool {
        let response: SuccessResponse = try await client.post(
            "/filters/subnets/exclude",
            body: SubnetFilterRequest(subnetIpAddress: subnetIpAddress, subnetMask: subnetMask)
        )
        return response.success
    }

    /// Allows a previously excluded host.
    /// `POST /filters/hosts/allow`
    func allowHost(ipAddress: String) async throws -> Bool {
        let response: SuccessResponse = try await client.post(
            "/filters/hosts/allow",
            body: HostFilterRequest(ipAddress: ipAddress)
        )
        return response.success
    }

    /// Allows a previously excluded subnet.
    /// `POST /filters/subnets/allow`
    func allowSubnet(subnetIpAddress: String, subnetMask: String) async throws -> Bool {
        let response: SuccessResponse = try await client.post(
            "/filters/subnets/allow",
            body: SubnetFilterRequest(subnetIpAddress: subnetIpAddress, subnetMask: subnetMask)
        )
        return response.success
    }

    /// Lists all excluded hosts.
    /// `GET /filters/hosts`
    func listExcludedHosts() async throws -> [String] {
        let response: HostsResponse = try await client.get("/filters/hosts")
        return response.hosts
    }

    /// Lists all excluded subnets.
    /// `GET /filters/subnets`
    func listExcludedSubnets() async throws -> [Subnet] {
        let response: SubnetsResponse = try await client.get("/filters/subnets")
        return response.subnets
    }

    // MARK: - Groups

    /// Creates a new group.
    /// `POST /groups`
    func createGroup(name: String, description: String = "") async throws -> Int64 {
        let response: GroupIdResponse = try await client.post(
            "/groups",
            body: CreateGroupRequest(name: name, description: description)
        )
        return response.groupId
    }

    /// Deletes a group.
    /// `DELETE /groups/{groupId}`
    func deleteGroup(groupId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.delete("/groups/\(groupId)")
        return response.success
    }

    /// Lists all group IDs.
    /// `GET /groups`
    func listGroups() async throws -> [Int64] {
        let response: GroupIdsResponse = try await client.get("/groups")
        return response.groupIds
    }

    /// Gets detailed information about a group.
    /// `GET /groups/{groupId}`
    func getGroupInfo(groupId: Int64) async throws -> GroupInfo {
        try await client.get("/groups/\(groupId)")
    }

    /// Gets the default group information.
    /// `GET /groups/default`
    func getDefaultGroupInfo() async throws -> GroupInfo {
        try await client.get("/groups/default")
    }

    /// Gets the list of peer IDs in a group.
    /// `GET /groups/{groupId}/peers`
    func getGroupPeerList(groupId: Int64) async throws -> [Int64] {
        let response: PeerIdsResponse = try await client.get("/groups/\(groupId)/peers")
        return response.peerIds
    }

    /// Adds a peer to a group.
    /// `PUT /groups/{groupId}/peers/{peerId}`
    func addPeerToGroup(groupId: Int64, peerId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/groups/\(groupId)/peers/\(peerId)")
        return response.success
    }

    /// Removes a peer from a group.
    /// `DELETE /groups/{groupId}/peers/{peerId}`
    func removePeerFromGroup(groupId: Int64, peerId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.delete("/groups/\(groupId)/peers/\(peerId)")
        return response.success
    }

    // MARK: - Modules

    /// Registers a new module.
    /// `POST /modules`
    func registerModule(libraryPath: String, bootstrapSymbol: String) async throws -> Int64 {
        let response: ModuleIdResponse = try await client.post(
            "/modules",
            body: RegisterModuleRequest(libraryPath: libraryPath, bootstrapSymbol: bootstrapSymbol)
        )
        return response.moduleId
    }

    /// Unregisters a module.
    /// `DELETE /modules/{moduleId}`
    func unregisterModule(moduleId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.delete("/modules/\(moduleId)")
        return response.success
    }

    /// Loads a module.
    /// `PUT /modules/{moduleId}/load`
    func loadModule(moduleId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/modules/\(moduleId)/load")
        return response.success
    }

    /// Unloads a module.
    /// `PUT /modules/{moduleId}/unload`
    func unloadModule(moduleId: Int64) async throws -> Bool {
        let response: SuccessResponse = try await client.put("/modules/\(moduleId)/unload")
        return response.success
    }

    /// Lists all active module IDs.
    /// `GET /modules/active`
    func listActiveModules() async throws -> [Int64] {
        let response: ModuleIdsResponse = try await client.get("/modules/active")
        return response.moduleIds
    }

    /// Lists all module IDs.
    /// `GET /modules`
    func listAllModules() async throws -> [Int64] {
        let response: ModuleIdsResponse = try await client.get("/modules")
        return response.moduleIds
    }

    /// Gets detailed information about a module.
    /// `GET /modules/{moduleId}`
    func getModuleInfo(moduleId: Int64) async throws -> ModuleInfo {
        try await client.get("/modules/\(moduleId)")
    }

    // MARK: - Lifecycle

    /// Shuts down the underlying REST client.
    func shutdown() {
        Task { await client.shutdown() }
    }
}
