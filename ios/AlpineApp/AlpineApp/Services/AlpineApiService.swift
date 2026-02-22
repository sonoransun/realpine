import Foundation
import os

private let logger = Logger(subsystem: "com.sonoranpub.AlpineApp", category: "AlpineApiService")

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
        logger.debug("GET /status")
        do {
            let result: ServerStatus = try await client.get("/status")
            logger.debug("GET /status succeeded")
            return result
        } catch {
            logger.error("GET /status failed: \(error.localizedDescription)")
            throw error
        }
    }

    // MARK: - Queries

    /// Starts a new query and returns the query ID.
    /// `POST /queries`
    func startQuery(_ request: QueryRequest) async throws -> QueryResponse {
        logger.debug("POST /queries (queryString: \(request.queryString))")
        do {
            let result: QueryResponse = try await client.post("/queries", body: request)
            logger.debug("POST /queries succeeded (queryId: \(result.queryId))")
            return result
        } catch {
            logger.error("POST /queries failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Starts a new query with individual parameters.
    /// `POST /queries`
    func startQuery(
        queryString: String,
        groupName: String = "",
        autoHaltLimit: Int64 = 100,
        peerDescMax: Int64 = 50
    ) async throws -> Int64 {
        logger.debug("POST /queries (queryString: \(queryString), groupName: \(groupName))")
        let request = QueryRequest(
            queryString: queryString,
            groupName: groupName,
            autoHaltLimit: autoHaltLimit,
            peerDescMax: peerDescMax
        )
        do {
            let response: QueryResponse = try await client.post("/queries", body: request)
            logger.debug("POST /queries succeeded (queryId: \(response.queryId))")
            return response.queryId
        } catch {
            logger.error("POST /queries failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets the status of an active query.
    /// `GET /queries/{queryId}`
    func getQueryStatus(queryId: Int64) async throws -> QueryStatusResponse {
        logger.debug("GET /queries/\(queryId)")
        do {
            let result: QueryStatusResponse = try await client.get("/queries/\(queryId)")
            logger.debug("GET /queries/\(queryId) succeeded")
            return result
        } catch {
            logger.error("GET /queries/\(queryId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets the results of a query.
    /// `GET /queries/{queryId}/results`
    func getQueryResults(queryId: Int64) async throws -> QueryResultsResponse {
        logger.debug("GET /queries/\(queryId)/results")
        do {
            let result: QueryResultsResponse = try await client.get("/queries/\(queryId)/results")
            logger.debug("GET /queries/\(queryId)/results succeeded")
            return result
        } catch {
            logger.error("GET /queries/\(queryId)/results failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Pauses an active query.
    /// `PUT /queries/{queryId}/pause`
    func pauseQuery(queryId: Int64) async throws -> Bool {
        logger.debug("PUT /queries/\(queryId)/pause")
        do {
            let response: SuccessResponse = try await client.put("/queries/\(queryId)/pause")
            logger.debug("PUT /queries/\(queryId)/pause succeeded")
            return response.success
        } catch {
            logger.error("PUT /queries/\(queryId)/pause failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Resumes a paused query.
    /// `PUT /queries/{queryId}/resume`
    func resumeQuery(queryId: Int64) async throws -> Bool {
        logger.debug("PUT /queries/\(queryId)/resume")
        do {
            let response: SuccessResponse = try await client.put("/queries/\(queryId)/resume")
            logger.debug("PUT /queries/\(queryId)/resume succeeded")
            return response.success
        } catch {
            logger.error("PUT /queries/\(queryId)/resume failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Cancels an active query.
    /// `DELETE /queries/{queryId}`
    func cancelQuery(queryId: Int64) async throws -> Bool {
        logger.debug("DELETE /queries/\(queryId)")
        do {
            let response: SuccessResponse = try await client.delete("/queries/\(queryId)")
            logger.debug("DELETE /queries/\(queryId) succeeded")
            return response.success
        } catch {
            logger.error("DELETE /queries/\(queryId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    // MARK: - Peers

    /// Gets all peer IDs.
    /// `GET /peers`
    func getAllPeers() async throws -> [Int64] {
        logger.debug("GET /peers")
        do {
            let response: PeerIdsResponse = try await client.get("/peers")
            logger.debug("GET /peers succeeded (\(response.peerIds.count) peers)")
            return response.peerIds
        } catch {
            logger.error("GET /peers failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets detailed information about a peer.
    /// `GET /peers/{peerId}`
    func getPeer(peerId: Int64) async throws -> PeerDetail {
        logger.debug("GET /peers/\(peerId)")
        do {
            let result: PeerDetail = try await client.get("/peers/\(peerId)")
            logger.debug("GET /peers/\(peerId) succeeded")
            return result
        } catch {
            logger.error("GET /peers/\(peerId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Adds a new peer.
    /// `POST /peers`
    func addPeer(ipAddress: String, port: Int64) async throws -> Bool {
        logger.debug("POST /peers (ip: \(ipAddress), port: \(port))")
        do {
            let response: SuccessResponse = try await client.post(
                "/peers",
                body: AddPeerRequest(ipAddress: ipAddress, port: port)
            )
            logger.debug("POST /peers succeeded")
            return response.success
        } catch {
            logger.error("POST /peers failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Looks up a peer ID by IP address and port.
    /// `GET /peers/lookup?ip={ip}&port={port}`
    func getPeerId(ipAddress: String, port: Int64) async throws -> Int64 {
        logger.debug("GET /peers/lookup (ip: \(ipAddress), port: \(port))")
        do {
            let response: PeerIdResponse = try await client.get(
                "/peers/lookup",
                queryItems: [
                    URLQueryItem(name: "ip", value: ipAddress),
                    URLQueryItem(name: "port", value: String(port))
                ]
            )
            logger.debug("GET /peers/lookup succeeded (peerId: \(response.peerId))")
            return response.peerId
        } catch {
            logger.error("GET /peers/lookup failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Activates a peer.
    /// `PUT /peers/{peerId}/activate`
    func activatePeer(peerId: Int64) async throws -> Bool {
        logger.debug("PUT /peers/\(peerId)/activate")
        do {
            let response: SuccessResponse = try await client.put("/peers/\(peerId)/activate")
            logger.debug("PUT /peers/\(peerId)/activate succeeded")
            return response.success
        } catch {
            logger.error("PUT /peers/\(peerId)/activate failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Deactivates a peer.
    /// `PUT /peers/{peerId}/deactivate`
    func deactivatePeer(peerId: Int64) async throws -> Bool {
        logger.debug("PUT /peers/\(peerId)/deactivate")
        do {
            let response: SuccessResponse = try await client.put("/peers/\(peerId)/deactivate")
            logger.debug("PUT /peers/\(peerId)/deactivate succeeded")
            return response.success
        } catch {
            logger.error("PUT /peers/\(peerId)/deactivate failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Pings a peer to check connectivity.
    /// `POST /peers/{peerId}/ping`
    func pingPeer(peerId: Int64) async throws -> Bool {
        logger.debug("POST /peers/\(peerId)/ping")
        do {
            let response: SuccessResponse = try await client.post("/peers/\(peerId)/ping")
            logger.debug("POST /peers/\(peerId)/ping succeeded")
            return response.success
        } catch {
            logger.error("POST /peers/\(peerId)/ping failed: \(error.localizedDescription)")
            throw error
        }
    }

    // MARK: - Filters

    /// Excludes a host from network operations.
    /// `POST /filters/hosts/exclude`
    func excludeHost(ipAddress: String) async throws -> Bool {
        logger.debug("POST /filters/hosts/exclude (ip: \(ipAddress))")
        do {
            let response: SuccessResponse = try await client.post(
                "/filters/hosts/exclude",
                body: HostFilterRequest(ipAddress: ipAddress)
            )
            logger.debug("POST /filters/hosts/exclude succeeded")
            return response.success
        } catch {
            logger.error("POST /filters/hosts/exclude failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Excludes a subnet from network operations.
    /// `POST /filters/subnets/exclude`
    func excludeSubnet(subnetIpAddress: String, subnetMask: String) async throws -> Bool {
        logger.debug("POST /filters/subnets/exclude (subnet: \(subnetIpAddress)/\(subnetMask))")
        do {
            let response: SuccessResponse = try await client.post(
                "/filters/subnets/exclude",
                body: SubnetFilterRequest(subnetIpAddress: subnetIpAddress, subnetMask: subnetMask)
            )
            logger.debug("POST /filters/subnets/exclude succeeded")
            return response.success
        } catch {
            logger.error("POST /filters/subnets/exclude failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Allows a previously excluded host.
    /// `POST /filters/hosts/allow`
    func allowHost(ipAddress: String) async throws -> Bool {
        logger.debug("POST /filters/hosts/allow (ip: \(ipAddress))")
        do {
            let response: SuccessResponse = try await client.post(
                "/filters/hosts/allow",
                body: HostFilterRequest(ipAddress: ipAddress)
            )
            logger.debug("POST /filters/hosts/allow succeeded")
            return response.success
        } catch {
            logger.error("POST /filters/hosts/allow failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Allows a previously excluded subnet.
    /// `POST /filters/subnets/allow`
    func allowSubnet(subnetIpAddress: String, subnetMask: String) async throws -> Bool {
        logger.debug("POST /filters/subnets/allow (subnet: \(subnetIpAddress)/\(subnetMask))")
        do {
            let response: SuccessResponse = try await client.post(
                "/filters/subnets/allow",
                body: SubnetFilterRequest(subnetIpAddress: subnetIpAddress, subnetMask: subnetMask)
            )
            logger.debug("POST /filters/subnets/allow succeeded")
            return response.success
        } catch {
            logger.error("POST /filters/subnets/allow failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Lists all excluded hosts.
    /// `GET /filters/hosts`
    func listExcludedHosts() async throws -> [String] {
        logger.debug("GET /filters/hosts")
        do {
            let response: HostsResponse = try await client.get("/filters/hosts")
            logger.debug("GET /filters/hosts succeeded (\(response.hosts.count) hosts)")
            return response.hosts
        } catch {
            logger.error("GET /filters/hosts failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Lists all excluded subnets.
    /// `GET /filters/subnets`
    func listExcludedSubnets() async throws -> [Subnet] {
        logger.debug("GET /filters/subnets")
        do {
            let response: SubnetsResponse = try await client.get("/filters/subnets")
            logger.debug("GET /filters/subnets succeeded (\(response.subnets.count) subnets)")
            return response.subnets
        } catch {
            logger.error("GET /filters/subnets failed: \(error.localizedDescription)")
            throw error
        }
    }

    // MARK: - Groups

    /// Creates a new group.
    /// `POST /groups`
    func createGroup(name: String, description: String = "") async throws -> Int64 {
        logger.debug("POST /groups (name: \(name))")
        do {
            let response: GroupIdResponse = try await client.post(
                "/groups",
                body: CreateGroupRequest(name: name, description: description)
            )
            logger.debug("POST /groups succeeded (groupId: \(response.groupId))")
            return response.groupId
        } catch {
            logger.error("POST /groups failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Deletes a group.
    /// `DELETE /groups/{groupId}`
    func deleteGroup(groupId: Int64) async throws -> Bool {
        logger.debug("DELETE /groups/\(groupId)")
        do {
            let response: SuccessResponse = try await client.delete("/groups/\(groupId)")
            logger.debug("DELETE /groups/\(groupId) succeeded")
            return response.success
        } catch {
            logger.error("DELETE /groups/\(groupId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Lists all group IDs.
    /// `GET /groups`
    func listGroups() async throws -> [Int64] {
        logger.debug("GET /groups")
        do {
            let response: GroupIdsResponse = try await client.get("/groups")
            logger.debug("GET /groups succeeded (\(response.groupIds.count) groups)")
            return response.groupIds
        } catch {
            logger.error("GET /groups failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets detailed information about a group.
    /// `GET /groups/{groupId}`
    func getGroupInfo(groupId: Int64) async throws -> GroupInfo {
        logger.debug("GET /groups/\(groupId)")
        do {
            let result: GroupInfo = try await client.get("/groups/\(groupId)")
            logger.debug("GET /groups/\(groupId) succeeded")
            return result
        } catch {
            logger.error("GET /groups/\(groupId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets the default group information.
    /// `GET /groups/default`
    func getDefaultGroupInfo() async throws -> GroupInfo {
        logger.debug("GET /groups/default")
        do {
            let result: GroupInfo = try await client.get("/groups/default")
            logger.debug("GET /groups/default succeeded")
            return result
        } catch {
            logger.error("GET /groups/default failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets the list of peer IDs in a group.
    /// `GET /groups/{groupId}/peers`
    func getGroupPeerList(groupId: Int64) async throws -> [Int64] {
        logger.debug("GET /groups/\(groupId)/peers")
        do {
            let response: PeerIdsResponse = try await client.get("/groups/\(groupId)/peers")
            logger.debug("GET /groups/\(groupId)/peers succeeded (\(response.peerIds.count) peers)")
            return response.peerIds
        } catch {
            logger.error("GET /groups/\(groupId)/peers failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Adds a peer to a group.
    /// `PUT /groups/{groupId}/peers/{peerId}`
    func addPeerToGroup(groupId: Int64, peerId: Int64) async throws -> Bool {
        logger.debug("PUT /groups/\(groupId)/peers/\(peerId)")
        do {
            let response: SuccessResponse = try await client.put("/groups/\(groupId)/peers/\(peerId)")
            logger.debug("PUT /groups/\(groupId)/peers/\(peerId) succeeded")
            return response.success
        } catch {
            logger.error("PUT /groups/\(groupId)/peers/\(peerId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Removes a peer from a group.
    /// `DELETE /groups/{groupId}/peers/{peerId}`
    func removePeerFromGroup(groupId: Int64, peerId: Int64) async throws -> Bool {
        logger.debug("DELETE /groups/\(groupId)/peers/\(peerId)")
        do {
            let response: SuccessResponse = try await client.delete("/groups/\(groupId)/peers/\(peerId)")
            logger.debug("DELETE /groups/\(groupId)/peers/\(peerId) succeeded")
            return response.success
        } catch {
            logger.error("DELETE /groups/\(groupId)/peers/\(peerId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    // MARK: - Modules

    /// Registers a new module.
    /// `POST /modules`
    func registerModule(libraryPath: String, bootstrapSymbol: String) async throws -> Int64 {
        logger.debug("POST /modules (libraryPath: \(libraryPath))")
        do {
            let response: ModuleIdResponse = try await client.post(
                "/modules",
                body: RegisterModuleRequest(libraryPath: libraryPath, bootstrapSymbol: bootstrapSymbol)
            )
            logger.debug("POST /modules succeeded (moduleId: \(response.moduleId))")
            return response.moduleId
        } catch {
            logger.error("POST /modules failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Unregisters a module.
    /// `DELETE /modules/{moduleId}`
    func unregisterModule(moduleId: Int64) async throws -> Bool {
        logger.debug("DELETE /modules/\(moduleId)")
        do {
            let response: SuccessResponse = try await client.delete("/modules/\(moduleId)")
            logger.debug("DELETE /modules/\(moduleId) succeeded")
            return response.success
        } catch {
            logger.error("DELETE /modules/\(moduleId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Loads a module.
    /// `PUT /modules/{moduleId}/load`
    func loadModule(moduleId: Int64) async throws -> Bool {
        logger.debug("PUT /modules/\(moduleId)/load")
        do {
            let response: SuccessResponse = try await client.put("/modules/\(moduleId)/load")
            logger.debug("PUT /modules/\(moduleId)/load succeeded")
            return response.success
        } catch {
            logger.error("PUT /modules/\(moduleId)/load failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Unloads a module.
    /// `PUT /modules/{moduleId}/unload`
    func unloadModule(moduleId: Int64) async throws -> Bool {
        logger.debug("PUT /modules/\(moduleId)/unload")
        do {
            let response: SuccessResponse = try await client.put("/modules/\(moduleId)/unload")
            logger.debug("PUT /modules/\(moduleId)/unload succeeded")
            return response.success
        } catch {
            logger.error("PUT /modules/\(moduleId)/unload failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Lists all active module IDs.
    /// `GET /modules/active`
    func listActiveModules() async throws -> [Int64] {
        logger.debug("GET /modules/active")
        do {
            let response: ModuleIdsResponse = try await client.get("/modules/active")
            logger.debug("GET /modules/active succeeded (\(response.moduleIds.count) modules)")
            return response.moduleIds
        } catch {
            logger.error("GET /modules/active failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Lists all module IDs.
    /// `GET /modules`
    func listAllModules() async throws -> [Int64] {
        logger.debug("GET /modules")
        do {
            let response: ModuleIdsResponse = try await client.get("/modules")
            logger.debug("GET /modules succeeded (\(response.moduleIds.count) modules)")
            return response.moduleIds
        } catch {
            logger.error("GET /modules failed: \(error.localizedDescription)")
            throw error
        }
    }

    /// Gets detailed information about a module.
    /// `GET /modules/{moduleId}`
    func getModuleInfo(moduleId: Int64) async throws -> ModuleInfo {
        logger.debug("GET /modules/\(moduleId)")
        do {
            let result: ModuleInfo = try await client.get("/modules/\(moduleId)")
            logger.debug("GET /modules/\(moduleId) succeeded")
            return result
        } catch {
            logger.error("GET /modules/\(moduleId) failed: \(error.localizedDescription)")
            throw error
        }
    }

    // MARK: - Lifecycle

    /// Shuts down the underlying REST client.
    func shutdown() {
        logger.info("Shutting down AlpineApiService")
        Task { await client.shutdown() }
    }
}
