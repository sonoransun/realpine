import Foundation

final class AlpineRpcService: Sendable {
    private let client: JsonRpcClient

    init(client: JsonRpcClient) {
        self.client = client
    }

    // MARK: - Status

    func getStatus() async throws -> ServerStatus {
        try await client.call(method: "getStatus")
    }

    // MARK: - Queries

    func startQuery(
        queryString: String,
        groupName: String = "",
        autoHaltLimit: Int64 = 100,
        peerDescMax: Int64 = 50
    ) async throws -> Int64 {
        let response: QueryResponse = try await client.call(
            method: "startQuery",
            params: [
                "queryString": queryString,
                "groupName": groupName,
                "autoHaltLimit": autoHaltLimit,
                "peerDescMax": peerDescMax
            ]
        )
        return response.queryId
    }

    func queryInProgress(queryId: Int64) async throws -> Bool {
        try await client.call(method: "queryInProgress", params: ["queryId": queryId])
    }

    func getQueryStatus(queryId: Int64) async throws -> QueryStatusResponse {
        try await client.call(method: "getQueryStatus", params: ["queryId": queryId])
    }

    func getQueryResults(queryId: Int64) async throws -> QueryResultsResponse {
        try await client.call(method: "getQueryResults", params: ["queryId": queryId])
    }

    func pauseQuery(queryId: Int64) async throws -> Bool {
        try await client.call(method: "pauseQuery", params: ["queryId": queryId])
    }

    func resumeQuery(queryId: Int64) async throws -> Bool {
        try await client.call(method: "resumeQuery", params: ["queryId": queryId])
    }

    func cancelQuery(queryId: Int64) async throws -> Bool {
        try await client.call(method: "cancelQuery", params: ["queryId": queryId])
    }

    // MARK: - Peers

    func getAllPeers() async throws -> [Int64] {
        try await client.call(method: "getAllPeers")
    }

    func getPeer(peerId: Int64) async throws -> PeerDetail {
        try await client.call(method: "getPeer", params: ["peerId": peerId])
    }

    func addPeer(ipAddress: String, port: Int64) async throws -> Bool {
        try await client.call(method: "addPeer", params: ["ipAddress": ipAddress, "port": port])
    }

    func getPeerId(ipAddress: String, port: Int64) async throws -> Int64 {
        try await client.call(method: "getPeerId", params: ["ipAddress": ipAddress, "port": port])
    }

    func activatePeer(peerId: Int64) async throws -> Bool {
        try await client.call(method: "activatePeer", params: ["peerId": peerId])
    }

    func deactivatePeer(peerId: Int64) async throws -> Bool {
        try await client.call(method: "deactivatePeer", params: ["peerId": peerId])
    }

    func pingPeer(peerId: Int64) async throws -> Bool {
        try await client.call(method: "pingPeer", params: ["peerId": peerId])
    }

    // MARK: - Filters

    func excludeHost(ipAddress: String) async throws -> Bool {
        try await client.call(method: "excludeHost", params: ["ipAddress": ipAddress])
    }

    func excludeSubnet(subnetIpAddress: String, subnetMask: String) async throws -> Bool {
        try await client.call(
            method: "excludeSubnet",
            params: ["subnetIpAddress": subnetIpAddress, "subnetMask": subnetMask]
        )
    }

    func allowHost(ipAddress: String) async throws -> Bool {
        try await client.call(method: "allowHost", params: ["ipAddress": ipAddress])
    }

    func allowSubnet(subnetIpAddress: String, subnetMask: String) async throws -> Bool {
        try await client.call(
            method: "allowSubnet",
            params: ["subnetIpAddress": subnetIpAddress, "subnetMask": subnetMask]
        )
    }

    func listExcludedHosts() async throws -> [String] {
        try await client.call(method: "listExcludedHosts")
    }

    func listExcludedSubnets() async throws -> [Subnet] {
        try await client.call(method: "listExcludedSubnets")
    }

    // MARK: - Groups

    func createGroup(name: String, description: String = "") async throws -> Int64 {
        try await client.call(
            method: "createGroup",
            params: ["name": name, "description": description]
        )
    }

    func deleteGroup(groupId: Int64) async throws -> Bool {
        try await client.call(method: "deleteGroup", params: ["groupId": groupId])
    }

    func listGroups() async throws -> [Int64] {
        try await client.call(method: "listGroups")
    }

    func getGroupInfo(groupId: Int64) async throws -> GroupInfo {
        try await client.call(method: "getGroupInfo", params: ["groupId": groupId])
    }

    func getDefaultGroupInfo() async throws -> GroupInfo {
        try await client.call(method: "getDefaultGroupInfo")
    }

    func getGroupPeerList(groupId: Int64) async throws -> [Int64] {
        try await client.call(method: "getGroupPeerList", params: ["groupId": groupId])
    }

    func addPeerToGroup(groupId: Int64, peerId: Int64) async throws -> Bool {
        try await client.call(
            method: "addPeerToGroup",
            params: ["groupId": groupId, "peerId": peerId]
        )
    }

    func removePeerFromGroup(groupId: Int64, peerId: Int64) async throws -> Bool {
        try await client.call(
            method: "removePeerFromGroup",
            params: ["groupId": groupId, "peerId": peerId]
        )
    }

    // MARK: - Modules

    func registerModule(libraryPath: String, bootstrapSymbol: String) async throws -> Int64 {
        try await client.call(
            method: "registerModule",
            params: ["libraryPath": libraryPath, "bootstrapSymbol": bootstrapSymbol]
        )
    }

    func unregisterModule(moduleId: Int64) async throws -> Bool {
        try await client.call(method: "unregisterModule", params: ["moduleId": moduleId])
    }

    func loadModule(moduleId: Int64) async throws -> Bool {
        try await client.call(method: "loadModule", params: ["moduleId": moduleId])
    }

    func unloadModule(moduleId: Int64) async throws -> Bool {
        try await client.call(method: "unloadModule", params: ["moduleId": moduleId])
    }

    func listActiveModules() async throws -> [Int64] {
        try await client.call(method: "listActiveModules")
    }

    func listAllModules() async throws -> [Int64] {
        try await client.call(method: "listAllModules")
    }

    func getModuleInfo(moduleId: Int64) async throws -> ModuleInfo {
        try await client.call(method: "getModuleInfo", params: ["moduleId": moduleId])
    }

    // MARK: - Lifecycle

    func shutdown() {
        Task { await client.shutdown() }
    }
}
