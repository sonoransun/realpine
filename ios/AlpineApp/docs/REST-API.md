# Alpine REST API Reference

## Overview

The Alpine REST API provides programmatic access to the Alpine peer-to-peer network server. It supports managing queries, peers, groups, modules, and network filters.

### Base URL

```
{scheme}://{host}:{port}/api/v1
```

- **scheme**: `http` or `https` (depending on TLS configuration)
- **host**: The Alpine server hostname or IP address
- **port**: The Alpine server port (default varies by deployment)

### Authentication

The API supports optional Bearer token authentication. When an API key is configured, include it in every request:

```
Authorization: Bearer <api-key>
```

If no API key is configured, the `Authorization` header should be omitted.

### Content Type

All request and response bodies use JSON:

```
Content-Type: application/json
Accept: application/json
```

### Versioning

The API is versioned via the URL path prefix `/api/v1`. Future breaking changes will use a new version prefix (e.g., `/api/v2`).

---

## Error Handling

### HTTP Status Codes

| Code | Meaning                  | When Returned                                      |
|------|--------------------------|----------------------------------------------------|
| 200  | OK                       | Request succeeded                                  |
| 400  | Bad Request              | Malformed request body or invalid parameters       |
| 401  | Unauthorized             | Missing or invalid API key                         |
| 404  | Not Found                | Resource does not exist (peer, group, module, etc.) |
| 500  | Internal Server Error    | Unexpected server-side failure                     |

### Error Response Format

Non-2xx responses include a JSON body with an error message:

```json
{
  "error": "Descriptive error message"
}
```

### Client-Side Error Mapping

The iOS client maps errors to the `ApiError` enum:

| ApiError Case           | Trigger                                              |
|-------------------------|------------------------------------------------------|
| `httpError(code, msg)`  | Server returned a non-2xx HTTP status code           |
| `invalidResponse`       | Response was not a valid `HTTPURLResponse`           |
| `encodingFailed`        | Request body could not be serialized to JSON         |
| `decodingFailed(error)` | Response body could not be deserialized from JSON    |
| `noData`                | Empty response when data was expected                |
| `networkError(error)`   | Network connectivity failure (timeout, no internet)  |
| `invalidURL(path)`      | Could not construct a valid URL from the given path  |

---

## Data Types

### Core Types

#### ServerStatus
```json
{
  "status": "running",
  "version": "2.0.0"
}
```

#### Subnet
```json
{
  "ipAddress": "192.168.0.0",
  "netMask": "255.255.255.0"
}
```

### Query Types

#### QueryRequest
```json
{
  "queryString": "*.mp3",
  "groupName": "",
  "autoHaltLimit": 100,
  "peerDescMax": 50
}
```
- `queryString` (required): Search pattern
- `groupName` (optional, default `""`): Restrict query to a specific group
- `autoHaltLimit` (optional, default `100`): Maximum peers to query before auto-halting
- `peerDescMax` (optional, default `50`): Maximum resource descriptions per peer

#### QueryResponse
```json
{
  "queryId": 12345
}
```

#### QueryStatusResponse
```json
{
  "inProgress": true,
  "totalPeers": 50,
  "peersQueried": 25,
  "numPeerResponses": 15,
  "totalHits": 120
}
```

#### QueryResultsResponse
```json
{
  "peers": [
    {
      "peerId": 1,
      "resources": [
        {
          "resourceId": 100,
          "size": 4096,
          "locators": ["http://10.0.0.1:8080/files/100"],
          "description": "document.pdf"
        }
      ]
    }
  ]
}
```

### Peer Types

#### PeerDetail
```json
{
  "peerId": 7,
  "ipAddress": "10.0.0.1",
  "port": 9000,
  "lastRecvTime": 1708617600,
  "lastSendTime": 1708617500,
  "avgBandwidth": 5000,
  "peakBandwidth": 10000
}
```

### Group Types

#### GroupInfo
```json
{
  "groupId": 1,
  "groupName": "default",
  "description": "Default peer group",
  "numPeers": 5,
  "totalQueries": 100,
  "totalResponses": 250
}
```

### Module Types

#### ModuleInfo
```json
{
  "moduleId": 77,
  "moduleName": "SearchIndex",
  "description": "Full-text search indexer",
  "version": "1.2.0",
  "libraryPath": "/usr/lib/alpine/mod_search.so",
  "bootstrapSymbol": "alpine_init",
  "activeTime": 3600
}
```

### Common Response Types

#### SuccessResponse
```json
{
  "success": true
}
```

#### ID Response Types
```json
{ "peerId": 42 }
{ "groupId": 10 }
{ "moduleId": 77 }
```

#### ID List Response Types
```json
{ "peerIds": [1, 2, 3] }
{ "groupIds": [10, 20] }
{ "moduleIds": [1, 3, 7] }
```

---

## Endpoints

### Status

#### Get Server Status

Returns the current status and version of the Alpine server.

```
GET /status
```

**Response** `200 OK`
```json
{
  "status": "running",
  "version": "2.0.0"
}
```

---

### Queries

#### Start Query

Initiates a new search query across the peer network.

```
POST /queries
```

**Request Body**
```json
{
  "queryString": "*.mp3",
  "groupName": "music",
  "autoHaltLimit": 200,
  "peerDescMax": 25
}
```

| Field           | Type   | Required | Default | Description                            |
|-----------------|--------|----------|---------|----------------------------------------|
| queryString     | String | Yes      | -       | Search pattern to match resources      |
| groupName       | String | No       | `""`    | Restrict to peers in this group        |
| autoHaltLimit   | Int64  | No       | `100`   | Max peers to query before auto-halt    |
| peerDescMax     | Int64  | No       | `50`    | Max resource descriptions per peer     |

**Response** `200 OK`
```json
{
  "queryId": 12345
}
```

**Errors**
- `400` - Invalid query string or parameters
- `500` - Server failed to create query

---

#### Get Query Status

Returns the current execution status of a query.

```
GET /queries/{queryId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| queryId   | Int64 | ID of the query      |

**Response** `200 OK`
```json
{
  "inProgress": true,
  "totalPeers": 50,
  "peersQueried": 25,
  "numPeerResponses": 15,
  "totalHits": 120
}
```

**Errors**
- `404` - Query not found

---

#### Get Query Results

Returns the resources found by a query.

```
GET /queries/{queryId}/results
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| queryId   | Int64 | ID of the query      |

**Response** `200 OK`
```json
{
  "peers": [
    {
      "peerId": 1,
      "resources": [
        {
          "resourceId": 100,
          "size": 4096,
          "locators": ["http://10.0.0.1:8080/files/100"],
          "description": "song.mp3"
        }
      ]
    }
  ]
}
```

**Errors**
- `404` - Query not found

---

#### Pause Query

Pauses an active query. The query can be resumed later.

```
PUT /queries/{queryId}/pause
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| queryId   | Int64 | ID of the query      |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Query not found
- `400` - Query is not in a pausable state

---

#### Resume Query

Resumes a previously paused query.

```
PUT /queries/{queryId}/resume
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| queryId   | Int64 | ID of the query      |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Query not found
- `400` - Query is not paused

---

#### Cancel Query

Cancels an active query and releases its resources.

```
DELETE /queries/{queryId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| queryId   | Int64 | ID of the query      |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Query not found

---

### Peers

#### List All Peers

Returns the IDs of all known peers.

```
GET /peers
```

**Response** `200 OK`
```json
{
  "peerIds": [1, 2, 3, 7, 12]
}
```

---

#### Get Peer Detail

Returns detailed information about a specific peer.

```
GET /peers/{peerId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| peerId    | Int64 | ID of the peer       |

**Response** `200 OK`
```json
{
  "peerId": 7,
  "ipAddress": "10.0.0.1",
  "port": 9000,
  "lastRecvTime": 1708617600,
  "lastSendTime": 1708617500,
  "avgBandwidth": 5000,
  "peakBandwidth": 10000
}
```

| Field         | Type   | Description                                 |
|---------------|--------|---------------------------------------------|
| peerId        | Int64  | Unique peer identifier                      |
| ipAddress     | String | Peer's IP address                           |
| port          | Int64  | Peer's port number                          |
| lastRecvTime  | Int64  | Timestamp of last received data (epoch sec) |
| lastSendTime  | Int64  | Timestamp of last sent data (epoch sec)     |
| avgBandwidth  | Int64  | Average bandwidth (bytes/sec)               |
| peakBandwidth | Int64  | Peak bandwidth observed (bytes/sec)         |

**Errors**
- `404` - Peer not found

---

#### Add Peer

Manually adds a new peer to the network by IP and port.

```
POST /peers
```

**Request Body**
```json
{
  "ipAddress": "192.168.1.100",
  "port": 8080
}
```

| Field     | Type   | Required | Description            |
|-----------|--------|----------|------------------------|
| ipAddress | String | Yes      | Peer's IP address      |
| port      | Int64  | Yes      | Peer's port number     |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `400` - Invalid IP address or port

---

#### Look Up Peer ID

Resolves a peer ID from an IP address and port.

```
GET /peers/lookup?ip={ipAddress}&port={port}
```

**Query Parameters**
| Parameter | Type   | Required | Description            |
|-----------|--------|----------|------------------------|
| ip        | String | Yes      | Peer's IP address      |
| port      | String | Yes      | Peer's port number     |

**Response** `200 OK`
```json
{
  "peerId": 42
}
```

**Errors**
- `404` - No peer found with the given address

---

#### Activate Peer

Activates a peer, allowing it to participate in queries and data exchange.

```
PUT /peers/{peerId}/activate
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| peerId    | Int64 | ID of the peer       |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Peer not found

---

#### Deactivate Peer

Deactivates a peer, excluding it from queries and data exchange.

```
PUT /peers/{peerId}/deactivate
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| peerId    | Int64 | ID of the peer       |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Peer not found

---

#### Ping Peer

Sends a connectivity check to a peer and reports whether it responded.

```
POST /peers/{peerId}/ping
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| peerId    | Int64 | ID of the peer       |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Peer not found

---

### Groups

#### List Groups

Returns the IDs of all groups.

```
GET /groups
```

**Response** `200 OK`
```json
{
  "groupIds": [1, 2, 5]
}
```

---

#### Create Group

Creates a new peer group.

```
POST /groups
```

**Request Body**
```json
{
  "name": "research-cluster",
  "description": "Peers dedicated to research data"
}
```

| Field       | Type   | Required | Default | Description                |
|-------------|--------|----------|---------|----------------------------|
| name        | String | Yes      | -       | Unique group name          |
| description | String | No       | `""`    | Human-readable description |

**Response** `200 OK`
```json
{
  "groupId": 55
}
```

**Errors**
- `400` - Group name already exists or is invalid

---

#### Delete Group

Deletes a group. Peers in the group are not deleted.

```
DELETE /groups/{groupId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| groupId   | Int64 | ID of the group      |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Group not found

---

#### Get Group Info

Returns detailed information about a group.

```
GET /groups/{groupId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| groupId   | Int64 | ID of the group      |

**Response** `200 OK`
```json
{
  "groupId": 1,
  "groupName": "default",
  "description": "Default peer group",
  "numPeers": 5,
  "totalQueries": 100,
  "totalResponses": 250
}
```

| Field          | Type   | Description                                  |
|----------------|--------|----------------------------------------------|
| groupId        | Int64  | Unique group identifier                      |
| groupName      | String | Group name                                   |
| description    | String | Human-readable description                   |
| numPeers       | Int64  | Number of peers currently in the group       |
| totalQueries   | Int64  | Total queries executed against this group    |
| totalResponses | Int64  | Total responses received from group peers    |

**Errors**
- `404` - Group not found

---

#### Get Default Group Info

Returns information about the default group.

```
GET /groups/default
```

**Response** `200 OK`
```json
{
  "groupId": 0,
  "groupName": "default",
  "description": "",
  "numPeers": 12,
  "totalQueries": 500,
  "totalResponses": 1200
}
```

---

#### List Group Peers

Returns the IDs of all peers in a group.

```
GET /groups/{groupId}/peers
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| groupId   | Int64 | ID of the group      |

**Response** `200 OK`
```json
{
  "peerIds": [1, 5, 9, 14]
}
```

**Errors**
- `404` - Group not found

---

#### Add Peer to Group

Adds an existing peer to a group.

```
PUT /groups/{groupId}/peers/{peerId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| groupId   | Int64 | ID of the group      |
| peerId    | Int64 | ID of the peer       |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Group or peer not found

---

#### Remove Peer from Group

Removes a peer from a group. The peer itself is not deleted.

```
DELETE /groups/{groupId}/peers/{peerId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| groupId   | Int64 | ID of the group      |
| peerId    | Int64 | ID of the peer       |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Group or peer not found

---

### Modules

#### List All Modules

Returns the IDs of all registered modules.

```
GET /modules
```

**Response** `200 OK`
```json
{
  "moduleIds": [1, 2, 3, 4, 5]
}
```

---

#### List Active Modules

Returns the IDs of currently loaded (active) modules.

```
GET /modules/active
```

**Response** `200 OK`
```json
{
  "moduleIds": [1, 3]
}
```

---

#### Register Module

Registers a new module from a shared library.

```
POST /modules
```

**Request Body**
```json
{
  "libraryPath": "/usr/lib/alpine/mod_search.so",
  "bootstrapSymbol": "alpine_init"
}
```

| Field           | Type   | Required | Description                                        |
|-----------------|--------|----------|----------------------------------------------------|
| libraryPath     | String | Yes      | Filesystem path to the shared library              |
| bootstrapSymbol | String | Yes      | Symbol name of the module's initialization function |

**Response** `200 OK`
```json
{
  "moduleId": 77
}
```

**Errors**
- `400` - Invalid library path or symbol
- `500` - Failed to load shared library

---

#### Unregister Module

Unregisters a module. The module must be unloaded first.

```
DELETE /modules/{moduleId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| moduleId  | Int64 | ID of the module     |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Module not found
- `400` - Module is still loaded

---

#### Get Module Info

Returns detailed information about a module.

```
GET /modules/{moduleId}
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| moduleId  | Int64 | ID of the module     |

**Response** `200 OK`
```json
{
  "moduleId": 77,
  "moduleName": "SearchIndex",
  "description": "Full-text search indexer",
  "version": "1.2.0",
  "libraryPath": "/usr/lib/alpine/mod_search.so",
  "bootstrapSymbol": "alpine_init",
  "activeTime": 3600
}
```

| Field           | Type   | Description                                    |
|-----------------|--------|------------------------------------------------|
| moduleId        | Int64  | Unique module identifier                       |
| moduleName      | String | Module display name                            |
| description     | String | Human-readable description                     |
| version         | String | Module version string                          |
| libraryPath     | String | Filesystem path to the shared library          |
| bootstrapSymbol | String | Initialization function symbol name            |
| activeTime      | Int64  | Seconds the module has been active (loaded)    |

**Errors**
- `404` - Module not found

---

#### Load Module

Loads a registered module, making it active.

```
PUT /modules/{moduleId}/load
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| moduleId  | Int64 | ID of the module     |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Module not found
- `400` - Module is already loaded

---

#### Unload Module

Unloads an active module without unregistering it.

```
PUT /modules/{moduleId}/unload
```

**Path Parameters**
| Parameter | Type  | Description          |
|-----------|-------|----------------------|
| moduleId  | Int64 | ID of the module     |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `404` - Module not found
- `400` - Module is not loaded

---

### Filters

Network filters control which hosts and subnets the server communicates with. Excluded hosts and subnets are blocked from all network operations.

#### List Excluded Hosts

Returns all currently excluded host IP addresses.

```
GET /filters/hosts
```

**Response** `200 OK`
```json
{
  "hosts": ["10.0.0.99", "192.168.1.200"]
}
```

---

#### Exclude Host

Adds a host IP address to the exclusion list.

```
POST /filters/hosts/exclude
```

**Request Body**
```json
{
  "ipAddress": "10.0.0.99"
}
```

| Field     | Type   | Required | Description                    |
|-----------|--------|----------|--------------------------------|
| ipAddress | String | Yes      | IP address of host to exclude  |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `400` - Invalid IP address

---

#### Allow Host

Removes a host IP address from the exclusion list.

```
POST /filters/hosts/allow
```

**Request Body**
```json
{
  "ipAddress": "10.0.0.99"
}
```

| Field     | Type   | Required | Description                  |
|-----------|--------|----------|------------------------------|
| ipAddress | String | Yes      | IP address of host to allow  |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `400` - Invalid IP address
- `404` - Host was not in the exclusion list

---

#### List Excluded Subnets

Returns all currently excluded subnets.

```
GET /filters/subnets
```

**Response** `200 OK`
```json
{
  "subnets": [
    {
      "ipAddress": "192.168.0.0",
      "netMask": "255.255.255.0"
    },
    {
      "ipAddress": "10.0.0.0",
      "netMask": "255.0.0.0"
    }
  ]
}
```

---

#### Exclude Subnet

Adds a subnet to the exclusion list.

```
POST /filters/subnets/exclude
```

**Request Body**
```json
{
  "subnetIpAddress": "192.168.0.0",
  "subnetMask": "255.255.255.0"
}
```

| Field           | Type   | Required | Description                       |
|-----------------|--------|----------|-----------------------------------|
| subnetIpAddress | String | Yes      | Network address of the subnet     |
| subnetMask      | String | Yes      | Subnet mask (dotted-decimal)      |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `400` - Invalid subnet address or mask

---

#### Allow Subnet

Removes a subnet from the exclusion list.

```
POST /filters/subnets/allow
```

**Request Body**
```json
{
  "subnetIpAddress": "192.168.0.0",
  "subnetMask": "255.255.255.0"
}
```

| Field           | Type   | Required | Description                       |
|-----------------|--------|----------|-----------------------------------|
| subnetIpAddress | String | Yes      | Network address of the subnet     |
| subnetMask      | String | Yes      | Subnet mask (dotted-decimal)      |

**Response** `200 OK`
```json
{
  "success": true
}
```

**Errors**
- `400` - Invalid subnet address or mask
- `404` - Subnet was not in the exclusion list

---

## Endpoint Summary

| Method | Endpoint                              | Description                    |
|--------|---------------------------------------|--------------------------------|
| GET    | `/status`                             | Get server status              |
| POST   | `/queries`                            | Start a new query              |
| GET    | `/queries/{queryId}`                  | Get query status               |
| GET    | `/queries/{queryId}/results`          | Get query results              |
| PUT    | `/queries/{queryId}/pause`            | Pause a query                  |
| PUT    | `/queries/{queryId}/resume`           | Resume a query                 |
| DELETE | `/queries/{queryId}`                  | Cancel a query                 |
| GET    | `/peers`                              | List all peer IDs              |
| GET    | `/peers/{peerId}`                     | Get peer details               |
| POST   | `/peers`                              | Add a new peer                 |
| GET    | `/peers/lookup`                       | Look up peer ID by address     |
| PUT    | `/peers/{peerId}/activate`            | Activate a peer                |
| PUT    | `/peers/{peerId}/deactivate`          | Deactivate a peer              |
| POST   | `/peers/{peerId}/ping`                | Ping a peer                    |
| GET    | `/groups`                             | List all group IDs             |
| POST   | `/groups`                             | Create a new group             |
| DELETE | `/groups/{groupId}`                   | Delete a group                 |
| GET    | `/groups/{groupId}`                   | Get group info                 |
| GET    | `/groups/default`                     | Get default group info         |
| GET    | `/groups/{groupId}/peers`             | List peers in a group          |
| PUT    | `/groups/{groupId}/peers/{peerId}`    | Add peer to group              |
| DELETE | `/groups/{groupId}/peers/{peerId}`    | Remove peer from group         |
| GET    | `/modules`                            | List all module IDs            |
| GET    | `/modules/active`                     | List active module IDs         |
| POST   | `/modules`                            | Register a module              |
| DELETE | `/modules/{moduleId}`                 | Unregister a module            |
| GET    | `/modules/{moduleId}`                 | Get module info                |
| PUT    | `/modules/{moduleId}/load`            | Load a module                  |
| PUT    | `/modules/{moduleId}/unload`          | Unload a module                |
| GET    | `/filters/hosts`                      | List excluded hosts            |
| POST   | `/filters/hosts/exclude`              | Exclude a host                 |
| POST   | `/filters/hosts/allow`                | Allow an excluded host         |
| GET    | `/filters/subnets`                    | List excluded subnets          |
| POST   | `/filters/subnets/exclude`            | Exclude a subnet               |
| POST   | `/filters/subnets/allow`              | Allow an excluded subnet       |

---

## iOS Client Architecture

The iOS client implements the REST API through three layers:

### RestApiClient (Actor)

Generic, actor-isolated HTTP client. Handles:
- URL construction from base URL + path
- Query parameter encoding
- JSON serialization/deserialization via `JSONEncoder`/`JSONDecoder`
- Bearer token authentication
- HTTP status code validation (2xx = success)
- Error mapping to `ApiError`

Provides overloaded methods for all HTTP verbs:
- `get<T>(_:queryItems:) -> T` and `get(_:queryItems:)` (void)
- `post<B,T>(_:body:) -> T`, `post<B>(_:body:)`, `post<T>(_:) -> T`, `post(_:)`
- `put<B,T>(_:body:) -> T`, `put<T>(_:) -> T`, `put(_:)`
- `delete<T>(_:) -> T` and `delete(_:)`

### AlpineApiService

Typed service layer providing named methods for every endpoint. Wraps `RestApiClient` calls with:
- Correct HTTP method and path for each endpoint
- Request body construction from method parameters
- Response unwrapping (e.g., extracting `queryId` from `QueryResponse`)

### RestApiTransport

Implements the `QueryTransport` protocol, delegating to `AlpineApiService`. Provides the query-specific subset of the API:
- `startQuery(_:) -> QueryResponse`
- `getQueryStatus(_:) -> QueryStatusResponse`
- `getQueryResults(_:) -> QueryResultsResponse`
- `cancelQuery(_:)`
- `shutdown()`

This enables the app's view models to work with any transport implementation (REST, JSON-RPC, broadcast) through the common protocol.
