import Testing
import Foundation
@testable import AlpineApp

@Suite("AlpineApiService Tests")
struct AlpineApiServiceTests {

    let service: AlpineApiService

    init() {
        URLProtocol.registerClass(MockURLProtocol.self)
        service = createMockApiService()
    }

    // MARK: - Status

    @Test("getStatus returns server status")
    func testGetStatus() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/status") == true)
            return try mockResponse(json: ["status": "running", "version": "2.0.0"])
        }

        let status = try await service.getStatus()
        #expect(status.status == "running")
        #expect(status.version == "2.0.0")
    }

    // MARK: - Queries

    @Test("startQuery with QueryRequest sends POST /queries and returns QueryResponse")
    func testStartQueryWithRequest() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/queries") == true)

            let body = try JSONDecoder().decode(QueryRequest.self, from: request.httpBody!)
            #expect(body.queryString == "test query")
            #expect(body.groupName == "mygroup")
            #expect(body.autoHaltLimit == 200)
            #expect(body.peerDescMax == 25)

            return try mockResponse(json: ["queryId": 99])
        }

        let req = QueryRequest(queryString: "test query", groupName: "mygroup", autoHaltLimit: 200, peerDescMax: 25)
        let response = try await service.startQuery(req)
        #expect(response.queryId == 99)
    }

    @Test("startQuery with parameters sends POST /queries and returns queryId")
    func testStartQueryWithParams() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/queries") == true)

            let body = try JSONDecoder().decode(QueryRequest.self, from: request.httpBody!)
            #expect(body.queryString == "*.txt")
            #expect(body.groupName == "")
            #expect(body.autoHaltLimit == 100)
            #expect(body.peerDescMax == 50)

            return try mockResponse(json: ["queryId": 77])
        }

        let queryId = try await service.startQuery(queryString: "*.txt")
        #expect(queryId == 77)
    }

    @Test("startQuery with custom parameters passes them correctly")
    func testStartQueryCustomParams() async throws {
        MockURLProtocol.requestHandler = { request in
            let body = try JSONDecoder().decode(QueryRequest.self, from: request.httpBody!)
            #expect(body.queryString == "*.mp3")
            #expect(body.groupName == "music")
            #expect(body.autoHaltLimit == 500)
            #expect(body.peerDescMax == 10)
            return try mockResponse(json: ["queryId": 42])
        }

        let queryId = try await service.startQuery(
            queryString: "*.mp3",
            groupName: "music",
            autoHaltLimit: 500,
            peerDescMax: 10
        )
        #expect(queryId == 42)
    }

    @Test("getQueryStatus sends GET /queries/{id}")
    func testGetQueryStatus() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.contains("/queries/42") == true)
            #expect(request.url?.path.contains("results") == false)
            return try mockResponse(json: [
                "inProgress": true,
                "totalPeers": 10,
                "peersQueried": 5,
                "numPeerResponses": 3,
                "totalHits": 15
            ])
        }

        let status = try await service.getQueryStatus(queryId: 42)
        #expect(status.inProgress == true)
        #expect(status.totalPeers == 10)
        #expect(status.peersQueried == 5)
        #expect(status.numPeerResponses == 3)
        #expect(status.totalHits == 15)
    }

    @Test("getQueryResults sends GET /queries/{id}/results")
    func testGetQueryResults() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.path.hasSuffix("/queries/42/results") == true)
            return try mockResponse(json: [
                "peers": [
                    [
                        "peerId": 1,
                        "resources": [
                            [
                                "resourceId": 1,
                                "size": 1024,
                                "locators": ["http://1.2.3.4/f"],
                                "description": "file.txt"
                            ]
                        ]
                    ]
                ]
            ])
        }

        let results = try await service.getQueryResults(queryId: 42)
        #expect(results.peers.count == 1)
        #expect(results.peers[0].peerId == 1)
        #expect(results.peers[0].resources.count == 1)
        #expect(results.peers[0].resources[0].resourceId == 1)
        #expect(results.peers[0].resources[0].size == 1024)
        #expect(results.peers[0].resources[0].locators == ["http://1.2.3.4/f"])
        #expect(results.peers[0].resources[0].description == "file.txt")
    }

    @Test("getQueryResults with multiple peers and resources")
    func testGetQueryResultsMultiple() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: [
                "peers": [
                    [
                        "peerId": 1,
                        "resources": [
                            ["resourceId": 10, "size": 512, "locators": [], "description": "a.txt"],
                            ["resourceId": 11, "size": 2048, "locators": ["loc1", "loc2"], "description": "b.mp3"]
                        ]
                    ],
                    [
                        "peerId": 2,
                        "resources": [
                            ["resourceId": 20, "size": 4096, "locators": ["loc3"], "description": "c.zip"]
                        ]
                    ]
                ]
            ])
        }

        let results = try await service.getQueryResults(queryId: 1)
        #expect(results.peers.count == 2)
        #expect(results.peers[0].resources.count == 2)
        #expect(results.peers[1].resources.count == 1)
        #expect(results.peers[1].resources[0].description == "c.zip")
    }

    @Test("getQueryResults with empty results")
    func testGetQueryResultsEmpty() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: ["peers": []])
        }

        let results = try await service.getQueryResults(queryId: 99)
        #expect(results.peers.isEmpty)
    }

    @Test("cancelQuery sends DELETE /queries/{id}")
    func testCancelQuery() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            #expect(request.url?.path.contains("/queries/42") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.cancelQuery(queryId: 42)
        #expect(result == true)
    }

    @Test("cancelQuery returns false on server rejection")
    func testCancelQueryFalse() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: ["success": false])
        }

        let result = try await service.cancelQuery(queryId: 99)
        #expect(result == false)
    }

    @Test("pauseQuery sends PUT /queries/{id}/pause")
    func testPauseQuery() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/queries/42/pause") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.pauseQuery(queryId: 42)
        #expect(result == true)
    }

    @Test("resumeQuery sends PUT /queries/{id}/resume")
    func testResumeQuery() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/queries/42/resume") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.resumeQuery(queryId: 42)
        #expect(result == true)
    }

    // MARK: - Peers

    @Test("getAllPeers sends GET /peers and returns peer IDs")
    func testGetAllPeers() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/peers") == true)
            return try mockResponse(json: ["peerIds": [1, 2, 3]])
        }

        let peers = try await service.getAllPeers()
        #expect(peers == [1, 2, 3])
    }

    @Test("getAllPeers returns empty array when no peers")
    func testGetAllPeersEmpty() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: ["peerIds": []])
        }

        let peers = try await service.getAllPeers()
        #expect(peers.isEmpty)
    }

    @Test("getPeer sends GET /peers/{id} and returns detail")
    func testGetPeer() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.path.contains("/peers/7") == true)
            return try mockResponse(json: [
                "peerId": 7,
                "ipAddress": "10.0.0.1",
                "port": 9000,
                "lastRecvTime": 100,
                "lastSendTime": 200,
                "avgBandwidth": 5000,
                "peakBandwidth": 10000
            ])
        }

        let peer = try await service.getPeer(peerId: 7)
        #expect(peer.peerId == 7)
        #expect(peer.ipAddress == "10.0.0.1")
        #expect(peer.port == 9000)
        #expect(peer.lastRecvTime == 100)
        #expect(peer.lastSendTime == 200)
        #expect(peer.avgBandwidth == 5000)
        #expect(peer.peakBandwidth == 10000)
    }

    @Test("addPeer sends POST /peers with body")
    func testAddPeer() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/peers") == true)
            let body = try JSONDecoder().decode(AddPeerRequest.self, from: request.httpBody!)
            #expect(body.ipAddress == "192.168.1.100")
            #expect(body.port == 8080)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.addPeer(ipAddress: "192.168.1.100", port: 8080)
        #expect(result == true)
    }

    @Test("getPeerId sends GET /peers/lookup with query params")
    func testGetPeerId() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            let components = URLComponents(url: request.url!, resolvingAgainstBaseURL: false)!
            let ip = components.queryItems?.first(where: { $0.name == "ip" })?.value
            let port = components.queryItems?.first(where: { $0.name == "port" })?.value
            #expect(ip == "10.0.0.5")
            #expect(port == "9090")
            return try mockResponse(json: ["peerId": 42])
        }

        let peerId = try await service.getPeerId(ipAddress: "10.0.0.5", port: 9090)
        #expect(peerId == 42)
    }

    @Test("activatePeer sends PUT /peers/{id}/activate")
    func testActivatePeer() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/peers/5/activate") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.activatePeer(peerId: 5)
        #expect(result == true)
    }

    @Test("deactivatePeer sends PUT /peers/{id}/deactivate")
    func testDeactivatePeer() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/peers/12/deactivate") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.deactivatePeer(peerId: 12)
        #expect(result == true)
    }

    @Test("pingPeer sends POST /peers/{id}/ping")
    func testPingPeer() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/peers/5/ping") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.pingPeer(peerId: 5)
        #expect(result == true)
    }

    // MARK: - Groups

    @Test("listGroups sends GET /groups and returns group IDs")
    func testListGroups() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/groups") == true)
            return try mockResponse(json: ["groupIds": [10, 20]])
        }

        let groups = try await service.listGroups()
        #expect(groups == [10, 20])
    }

    @Test("createGroup sends POST /groups and returns groupId")
    func testCreateGroup() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/groups") == true)
            let body = try JSONDecoder().decode(CreateGroupRequest.self, from: request.httpBody!)
            #expect(body.name == "TestGroup")
            #expect(body.description == "A test group")
            return try mockResponse(json: ["groupId": 55])
        }

        let groupId = try await service.createGroup(name: "TestGroup", description: "A test group")
        #expect(groupId == 55)
    }

    @Test("createGroup with default empty description")
    func testCreateGroupDefaultDescription() async throws {
        MockURLProtocol.requestHandler = { request in
            let body = try JSONDecoder().decode(CreateGroupRequest.self, from: request.httpBody!)
            #expect(body.name == "MinimalGroup")
            #expect(body.description == "")
            return try mockResponse(json: ["groupId": 10])
        }

        let groupId = try await service.createGroup(name: "MinimalGroup")
        #expect(groupId == 10)
    }

    @Test("deleteGroup sends DELETE /groups/{id}")
    func testDeleteGroup() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            #expect(request.url?.path.contains("/groups/55") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.deleteGroup(groupId: 55)
        #expect(result == true)
    }

    @Test("getGroupInfo sends GET /groups/{id}")
    func testGetGroupInfo() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.path.contains("/groups/1") == true)
            return try mockResponse(json: [
                "groupId": 1,
                "groupName": "default",
                "description": "Default group",
                "numPeers": 5,
                "totalQueries": 100,
                "totalResponses": 250
            ])
        }

        let info = try await service.getGroupInfo(groupId: 1)
        #expect(info.groupId == 1)
        #expect(info.groupName == "default")
        #expect(info.description == "Default group")
        #expect(info.numPeers == 5)
        #expect(info.totalQueries == 100)
        #expect(info.totalResponses == 250)
    }

    @Test("getDefaultGroupInfo sends GET /groups/default")
    func testGetDefaultGroupInfo() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.path.hasSuffix("/groups/default") == true)
            return try mockResponse(json: [
                "groupId": 0,
                "groupName": "default",
                "description": "",
                "numPeers": 3,
                "totalQueries": 50,
                "totalResponses": 120
            ])
        }

        let info = try await service.getDefaultGroupInfo()
        #expect(info.groupId == 0)
        #expect(info.groupName == "default")
        #expect(info.numPeers == 3)
    }

    @Test("getGroupPeerList sends GET /groups/{id}/peers")
    func testGetGroupPeerList() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/groups/10/peers") == true)
            return try mockResponse(json: ["peerIds": [1, 5, 9]])
        }

        let peers = try await service.getGroupPeerList(groupId: 10)
        #expect(peers == [1, 5, 9])
    }

    @Test("addPeerToGroup sends PUT /groups/{gid}/peers/{pid}")
    func testAddPeerToGroup() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/groups/10/peers/5") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.addPeerToGroup(groupId: 10, peerId: 5)
        #expect(result == true)
    }

    @Test("removePeerFromGroup sends DELETE /groups/{gid}/peers/{pid}")
    func testRemovePeerFromGroup() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            #expect(request.url?.path.hasSuffix("/groups/10/peers/5") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.removePeerFromGroup(groupId: 10, peerId: 5)
        #expect(result == true)
    }

    // MARK: - Modules

    @Test("registerModule sends POST /modules and returns moduleId")
    func testRegisterModule() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/modules") == true)
            let body = try JSONDecoder().decode(RegisterModuleRequest.self, from: request.httpBody!)
            #expect(body.libraryPath == "/lib/mod.so")
            #expect(body.bootstrapSymbol == "init")
            return try mockResponse(json: ["moduleId": 77])
        }

        let moduleId = try await service.registerModule(libraryPath: "/lib/mod.so", bootstrapSymbol: "init")
        #expect(moduleId == 77)
    }

    @Test("unregisterModule sends DELETE /modules/{id}")
    func testUnregisterModule() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "DELETE")
            #expect(request.url?.path.contains("/modules/77") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.unregisterModule(moduleId: 77)
        #expect(result == true)
    }

    @Test("getModuleInfo sends GET /modules/{id}")
    func testGetModuleInfo() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.url?.path.contains("/modules/77") == true)
            return try mockResponse(json: [
                "moduleId": 77,
                "moduleName": "TestModule",
                "description": "A module",
                "version": "1.0",
                "libraryPath": "/lib/mod.so",
                "bootstrapSymbol": "init",
                "activeTime": 3600
            ])
        }

        let info = try await service.getModuleInfo(moduleId: 77)
        #expect(info.moduleId == 77)
        #expect(info.moduleName == "TestModule")
        #expect(info.description == "A module")
        #expect(info.version == "1.0")
        #expect(info.libraryPath == "/lib/mod.so")
        #expect(info.bootstrapSymbol == "init")
        #expect(info.activeTime == 3600)
    }

    @Test("loadModule sends PUT /modules/{id}/load")
    func testLoadModule() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/modules/77/load") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.loadModule(moduleId: 77)
        #expect(result == true)
    }

    @Test("unloadModule sends PUT /modules/{id}/unload")
    func testUnloadModule() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "PUT")
            #expect(request.url?.path.hasSuffix("/modules/77/unload") == true)
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.unloadModule(moduleId: 77)
        #expect(result == true)
    }

    @Test("listActiveModules sends GET /modules/active")
    func testListActiveModules() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/modules/active") == true)
            return try mockResponse(json: ["moduleIds": [1, 3, 7]])
        }

        let moduleIds = try await service.listActiveModules()
        #expect(moduleIds == [1, 3, 7])
    }

    @Test("listAllModules sends GET /modules")
    func testListAllModules() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/modules") == true)
            return try mockResponse(json: ["moduleIds": [1, 2, 3, 4, 5]])
        }

        let moduleIds = try await service.listAllModules()
        #expect(moduleIds == [1, 2, 3, 4, 5])
    }

    // MARK: - Filters

    @Test("excludeHost sends POST /filters/hosts/exclude with body")
    func testExcludeHost() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/filters/hosts/exclude") == true)
            let body = try JSONDecoder().decode(HostFilterRequest.self, from: request.httpBody!)
            #expect(body.ipAddress == "10.0.0.99")
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.excludeHost(ipAddress: "10.0.0.99")
        #expect(result == true)
    }

    @Test("allowHost sends POST /filters/hosts/allow with body")
    func testAllowHost() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/filters/hosts/allow") == true)
            let body = try JSONDecoder().decode(HostFilterRequest.self, from: request.httpBody!)
            #expect(body.ipAddress == "10.0.0.99")
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.allowHost(ipAddress: "10.0.0.99")
        #expect(result == true)
    }

    @Test("listExcludedHosts sends GET /filters/hosts")
    func testListExcludedHosts() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "GET")
            #expect(request.url?.path.hasSuffix("/filters/hosts") == true)
            return try mockResponse(json: ["hosts": ["10.0.0.1", "10.0.0.2"]])
        }

        let hosts = try await service.listExcludedHosts()
        #expect(hosts == ["10.0.0.1", "10.0.0.2"])
    }

    @Test("listExcludedHosts returns empty when no exclusions")
    func testListExcludedHostsEmpty() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: ["hosts": []])
        }

        let hosts = try await service.listExcludedHosts()
        #expect(hosts.isEmpty)
    }

    @Test("excludeSubnet sends POST /filters/subnets/exclude with body")
    func testExcludeSubnet() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/filters/subnets/exclude") == true)
            let body = try JSONDecoder().decode(SubnetFilterRequest.self, from: request.httpBody!)
            #expect(body.subnetIpAddress == "192.168.0.0")
            #expect(body.subnetMask == "255.255.255.0")
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.excludeSubnet(subnetIpAddress: "192.168.0.0", subnetMask: "255.255.255.0")
        #expect(result == true)
    }

    @Test("allowSubnet sends POST /filters/subnets/allow with body")
    func testAllowSubnet() async throws {
        MockURLProtocol.requestHandler = { request in
            #expect(request.httpMethod == "POST")
            #expect(request.url?.path.hasSuffix("/filters/subnets/allow") == true)
            let body = try JSONDecoder().decode(SubnetFilterRequest.self, from: request.httpBody!)
            #expect(body.subnetIpAddress == "10.0.0.0")
            #expect(body.subnetMask == "255.0.0.0")
            return try mockResponse(json: ["success": true])
        }

        let result = try await service.allowSubnet(subnetIpAddress: "10.0.0.0", subnetMask: "255.0.0.0")
        #expect(result == true)
    }

    @Test("listExcludedSubnets sends GET /filters/subnets")
    func testListExcludedSubnets() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(json: [
                "subnets": [
                    ["ipAddress": "192.168.0.0", "netMask": "255.255.255.0"],
                    ["ipAddress": "10.0.0.0", "netMask": "255.0.0.0"]
                ]
            ])
        }

        let subnets = try await service.listExcludedSubnets()
        #expect(subnets.count == 2)
        #expect(subnets[0].ipAddress == "192.168.0.0")
        #expect(subnets[0].netMask == "255.255.255.0")
        #expect(subnets[1].ipAddress == "10.0.0.0")
        #expect(subnets[1].netMask == "255.0.0.0")
    }

    // MARK: - Error Propagation

    @Test("API service propagates HTTP errors from client")
    func testErrorPropagation() async throws {
        MockURLProtocol.requestHandler = { _ in
            try mockResponse(statusCode: 404, json: ["error": "Not found"])
        }

        do {
            _ = try await service.getStatus()
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .httpError(let code, _) = error {
                #expect(code == 404)
            } else {
                Issue.record("Expected httpError, got \(error)")
            }
        }
    }

    @Test("API service propagates decoding errors from client")
    func testDecodingErrorPropagation() async throws {
        MockURLProtocol.requestHandler = { _ in
            let data = "invalid".data(using: .utf8)!
            let response = HTTPURLResponse(
                url: URL(string: "http://localhost:8080/api/v1/status")!,
                statusCode: 200,
                httpVersion: "HTTP/1.1",
                headerFields: nil
            )!
            return (response, data)
        }

        do {
            _ = try await service.getStatus()
            Issue.record("Expected error")
        } catch let error as ApiError {
            if case .decodingFailed = error {
                // Expected
            } else {
                Issue.record("Expected decodingFailed, got \(error)")
            }
        }
    }
}
