# Alpine

A decentralized peer-to-peer resource discovery and distributed querying platform built in modern C++23.

Alpine enables autonomous nodes to discover each other, share resource metadata, and retrieve content across a network without relying on any central directory or coordinator. Peers locate one another through broadcast-based discovery, exchange queries in parallel, and aggregate results from across the network — all driven by adaptive quality metrics that learn which peers are most reliable over time.

---

## Table of Contents

- [How It Works](#how-it-works)
- [Use Cases](#use-cases)
- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [REST API Reference](#rest-api-reference)
- [CLI Reference](#cli-reference)
- [Protocol Stack](#protocol-stack)
- [Configuration](#configuration)
- [Security](#security)
- [Observability](#observability)
- [Deployment](#deployment)
- [Building](#building)
- [Testing](#testing)
- [Module Plugin System](#module-plugin-system)
- [FUSE Virtual Filesystem](#fuse-virtual-filesystem)
- [Platform Support](#platform-support)
- [License](#license)

---

## How It Works

### Decentralized Resource Discovery

Alpine's discovery model operates without a central index. Every node is both a producer and a consumer of resources:

1. **Broadcast discovery** — Nodes announce their presence via UDP broadcast and multicast. No registration server is required.
2. **Query fan-out** — When a query is submitted, the originating node fans it out to all known peers in parallel. Each peer checks its local resources and responds.
3. **Result aggregation** — Responses flow back and are aggregated in real time. Results can be streamed incrementally via Server-Sent Events or collected via webhooks.
4. **Adaptive quality** — A built-in rating engine tracks peer reliability using exponential moving averages, circuit breakers, and gossip-based score sharing. Unreliable peers are automatically deprioritized.
5. **Cluster coordination** — Nodes in the same cluster share heartbeats, deduplicate queries across the group, and federate searches to peer clusters — ensuring no wasted work and fast convergence.

```
                  ┌────────────┐
                  │   Client   │
                  │ (REST/CLI) │
                  └──────┬─────┘
                         │ POST /query
                         ▼
  ┌──────────────────────────────────────────┐
  │             Node A  (Bridge)             │
  │  ┌────────┐  ┌──────────┐  ┌──────────┐ │
  │  │ Router │→ │ QueryMgr │→ │ RatingEng│ │
  │  └────────┘  └────┬─────┘  └──────────┘ │
  └───────────────────┼──────────────────────┘
         queryDiscover│          queryDiscover
           ┌──────────┼──────────────┐
           ▼                         ▼
    ┌────────────┐            ┌────────────┐
    │   Node B   │            │   Node C   │
    │  (Peer)    │            │  (Peer)    │
    └──────┬─────┘            └──────┬─────┘
           │ queryOffer              │ queryOffer
           └──────────┐  ┌──────────┘
                      ▼  ▼
               Node A aggregates
            queryRequest → queryReply
                      │
                      ▼
              ┌──────────────┐
              │  Aggregated  │
              │   Results    │  → SSE stream / webhook / REST poll
              └──────────────┘
```

### Query Lifecycle

A distributed query proceeds through four protocol phases:

| Phase | Direction | Purpose |
|-------|-----------|---------|
| **Discover** | Originator → Peers | Broadcast query metadata (search terms, group, priority) |
| **Offer** | Peers → Originator | Each peer responds with its hit count for the query |
| **Request** | Originator → Peers | Originator selects best peers (by rating score) and requests full results |
| **Reply** | Peers → Originator | Selected peers send resource descriptors back |

The originator selects which peers to request from based on the `AlpineRatingEngine`'s per-peer quality scores. Peers with open circuit breakers are skipped entirely.

### Cluster Coordination

When multiple Alpine nodes share a network, the `ClusterCoordinator` provides:

- **Heartbeat-based membership** — Nodes broadcast UDP heartbeats every 10 seconds. Stale nodes (no heartbeat for 35-120 seconds, adaptive) are evicted.
- **Query deduplication** — Before starting a query, a node checks whether any cluster peer is already running the same search. Duplicate queries are redirected to the existing result set.
- **Load-aware routing** — Nodes report their CPU load, active query count, and connection count. Overloaded nodes redirect new queries to less-loaded peers.
- **Split-brain detection** — If more than 50% of known nodes become unreachable, the node flags itself as isolated and health probes return 503.
- **Federated search** — Queries can be forwarded across cluster boundaries, with results aggregated from remote nodes via HTTP.

```
  Region: us-west                        Region: eu-central
 ┌─────────────────────┐                ┌─────────────────────┐
 │  ┌───┐  ┌───┐       │  Heartbeats   │       ┌───┐  ┌───┐  │
 │  │ A │←→│ B │       │◄────────────► │       │ D │←→│ E │  │
 │  └─┬─┘  └─┬─┘       │               │       └─┬─┘  └─┬─┘  │
 │    └───┬───┘         │               │         └───┬───┘    │
 │      ┌─┴─┐           │  Federated    │           ┌─┴─┐     │
 │      │ C │           │  queries ────►│           │ F │     │
 │      └───┘           │               │           └───┘     │
 └─────────────────────┘                └─────────────────────┘
      3 nodes, 1 cluster                    3 nodes, 1 cluster
```

---

## Use Cases

### Media Sharing on a Home Network

A family has media files spread across a NAS, a laptop, and a Raspberry Pi. Each device runs an Alpine node with the DLNA service enabled. When someone searches for "vacation photos," the query is broadcast to all devices simultaneously. Each node checks its local content index and returns matching files. The results are merged and presented in a single list — no central media server required. Standard media players can browse the shared library directly through DLNA/UPnP.

Because Alpine tracks peer quality over time, a NAS that responds in 5ms will naturally rank higher than a Raspberry Pi that takes 200ms. Subsequent queries route to the faster peers first. If the Pi goes offline, the circuit breaker marks it as open and queries skip it entirely — no timeouts, no delays.

### Distributed Enterprise Search

An organization has data spread across departments — engineering docs, sales reports, HR records — each on a separate server. Alpine nodes on each server register their resources and join a common cluster.

When an authorized user queries from any endpoint, the search fans out to every department node. The RBAC policy engine checks the user's API key against their role: a `query`-role key can search and view results, but cannot access admin endpoints or ban peers. Results from all departments arrive in real time via SSE streaming, so the user sees matches incrementally rather than waiting for every node to finish.

The cluster coordinator ensures that if two users submit the same search simultaneously, only one actual query runs — the second user's request is deduplicated and served from the same result set. The rating engine learns which servers respond fastest, so future queries prioritize the most responsive nodes.

### IoT and Edge Service Discovery

In an industrial IoT deployment, edge devices need to discover available services (sensor data feeds, actuator controls, firmware update servers) without a central registry that becomes a single point of failure.

Alpine nodes on each device broadcast their capabilities via WiFi multicast. New devices are discovered automatically within seconds. Service metadata is exchanged through the standard query protocol — a control station can search for "temperature sensor" and find every matching device on the network.

If a sensor gateway goes offline, the circuit breaker transitions to Open state after 5 consecutive failures. After 30 seconds, it moves to HalfOpen and sends a single probe. If the device responds, the circuit closes and normal queries resume. If not, it stays open. This automatic recovery cycle means no manual intervention is needed when devices reboot or move between access points.

Queries carry priority levels (0-255), so a safety-critical alert query at priority 255 jumps ahead of routine telemetry queries at priority 128. The query manager processes higher-priority queries first at every hop.

### Privacy-Conscious File Sharing over Tor

Users who want to share files without exposing their IP addresses can enable Alpine's Tor integration. Each node registers as a Tor hidden service, and peer discovery happens through `.onion` addresses routed via the Tor network.

The `TorTunnel` module connects to configured peers through a SOCKS5 proxy, encrypting all traffic through multiple relay hops. The `TorHiddenService` module makes the node reachable to other Tor-enabled Alpine peers. Standard queries work identically — the only difference is the transport layer. Results are delivered through the same REST API or SSE stream.

### Multi-Region Kubernetes Deployment

A SaaS company runs Alpine clusters in three regions: US-West, US-East, and EU-Central. Each region has 3-10 nodes managed by Helm with horizontal pod autoscaling.

Within each region, nodes discover each other via UDP beacon broadcasts. The `ClusterCoordinator` tracks node health with RTT measurements and CPU load estimates. When a region receives a query that requires data from another region, it uses federated search to forward the request.

The Helm chart configures pod disruption budgets (max 1 unavailable) for zero-downtime rolling updates. Kubernetes readiness probes hit `/health/ready` to ensure traffic only routes to fully initialized nodes. If a pod fails its liveness probe (`/health/live`), Kubernetes restarts it automatically.

Prometheus scrapes `/metrics` from every pod, providing dashboards for request rates, query latencies (p50/p95/p99), peer connection counts, and circuit breaker state across the entire fleet.

---

## Quick Start

### Docker Compose (Recommended)

Launch a 3-node cluster:

```sh
docker-compose -f docker/docker-compose.yml up
```

Query the cluster:

```sh
# Check node status
curl http://localhost:8081/status | jq .

# Start a query
curl -X POST http://localhost:8081/query \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <api-key>" \
  -d '{"queryString": "example search", "priority": 200}'

# Get results (paginated)
curl http://localhost:8081/query/1/results?limit=50

# Stream results in real time
curl -N http://localhost:8081/query/1/stream \
  -H "Authorization: Bearer <api-key>"

# Check cluster health
curl http://localhost:8081/cluster/status | jq .
```

### Build from Source

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/AlpineRestBridge --logFile alpine.log --logLevel Info \
  --ipAddress 0.0.0.0 --port 9000 --restPort 8080
```

### CLI First Query

```sh
# Install completions (bash)
eval "$(./build/bin/AlpineCmdIf --completions bash)"

# Use the 'alpc' alias
alpc --serverAddress localhost --serverPort 9000 --command beginQuery \
  --queryString "hello world" --json
```

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        REST / CLI / GUI                         │
│  AlpineRestBridge      AlpineCmdIf         AlpineGui            │
│  (HTTP/SSE/WS)         (interactive)       (ImGui)              │
├─────────────────────────────────────────────────────────────────┤
│                       Interfaces Layer                          │
│  AlpineStackInterface          DtcpStackInterface               │
├─────────────────────────────────────────────────────────────────┤
│                    Application Core (ApplCore)                  │
│  Lifecycle · Signals · Configuration · Logging · Tracing        │
├─────────────────────────────────────────────────────────────────┤
│                    Protocol Layer (AlpineProtocol)              │
│  QueryMgr         PeerMgr          GroupMgr      ModuleMgr     │
│  RatingEngine     CircuitBreaker   PacketAuth    IpFilter       │
│  QueryCache       AlpineRatingEngine                            │
├─────────────────────────────────────────────────────────────────┤
│                   Transport Layer (AlpineTransport)             │
│  UDP Unicast · Multicast · Broadcast · WiFi Direct              │
│  DTCP Reliable · DTLS Wrapper · SendQueue                       │
├─────────────────────────────────────────────────────────────────┤
│                       Base Libraries                            │
│  AppUtils    SysUtils    ThreadUtils    NetUtils                 │
│  ConfigUtils VfsUtils    Tracing        Platform                 │
└─────────────────────────────────────────────────────────────────┘
```

### Key Binaries

| Binary | Purpose |
|--------|---------|
| `AlpineRestBridge` | Full-featured REST API server with cluster coordination, DLNA, mDNS, SSDP, Tor, Prometheus metrics |
| `AlpineServer` | Standalone server daemon with JSON-RPC interface |
| `AlpineCmdIf` | Interactive CLI client (`alpc` alias) with shell completions |

### Design Patterns

- **Static Facade** — Major classes (`AlpineStackInterface`, `ApplCore`, `Configuration`, `Log`) are pure static with thread-safe static members. No instantiation required.
- **RAII Guards** — `ConnectionGuard` for HTTP connections, `ReadLock`/`WriteLock` for shared state, `unique_ptr` with custom deleters for system resources.
- **Sharded Locking** — IP rate limiting (16 shards), query deduplication (8 shards), and connection tracking use shard arrays for low contention under high concurrency.
- **Dynamic Thread Pool** — HTTP server scales from 4 to 32 worker threads based on load. Idle threads exit after 30 seconds; new threads spawn when all workers are busy.
- **Circuit Breaker** — Per-peer fault tolerance with Closed/Open/HalfOpen state machine. Configurable failure thresholds and recovery timeouts prevent cascading failures.
- **Protocol Versioning** — Zero-marker format allows new nodes to detect version while old nodes gracefully ignore unknown packets.

### Service Initialization Order

`AlpineRestBridge` starts services in this sequence, with each optional service following the non-fatal pattern (log and continue on failure):

```
1. ApplCore + Configuration
2. Alpine Stack (protocol + transport)
3. REST route registration (QueryHandler, PeerHandler, StatusHandler, AuthHandler,
   MetricsHandler, AdminHandler, ClusterCoordinator, ApiDocsHandler)
4. API key authentication middleware
5. OpenTelemetry tracing (optional)
6. FUSE virtual filesystem (optional)
7. Discovery beacon (UDP broadcast)
8. Cluster coordinator (heartbeats + listener)
9. Broadcast query handler
10. WiFi multicast discovery
11. UPnP port mapping (optional)
12. Tor hidden service + tunnel (optional)
13. DLNA media server + SSDP + mDNS (optional)
14. HTTP server (blocking accept loop)
```

Shutdown proceeds in reverse order with a configurable drain period (default 5 seconds) for in-flight requests.

---

## REST API Reference

All endpoints require API key authentication via `Authorization: Bearer <key>` header unless noted.

### Query Operations

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/query` | Start a distributed query. Body: `{queryString, groupName?, priority?, numHits?, callbackUrl?}`. Returns 202 with `{queryId}`. Priority 0-255, default 128. |
| `GET` | `/query/:id` | Get query status: `{queryId, inProgress, totalPeers, peersQueried, numPeerResponses, totalHits}`. |
| `GET` | `/query/:id/results` | Get paginated results. Params: `limit` (max 1000), `offset`. Returns `{data[], total, hasMore}`. |
| `GET` | `/query/:id/stream` | Server-Sent Events stream. Events: `result` (per-peer), `progress` (live stats), `complete` (final). |
| `DELETE` | `/query/:id` | Cancel a running query. |

**Query priority**: 0 (lowest) to 255 (highest), default 128. Higher-priority queries are processed first at every node in the chain.

**Webhooks**: Include `callbackUrl` in the POST body. On completion, Alpine POSTs results to the URL with an `X-Alpine-Signature` HMAC-SHA256 header for verification. Retries with exponential backoff on failure.

### Peer Operations

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/peers` | List all peers. Params: `limit`, `offset`, `status` (active/inactive), `minScore`. |
| `GET` | `/peers/:id` | Get peer details: IP, port, bandwidth, timing stats, circuit breaker state. |

### Status & Health

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/status` | Server status: version, uptime, peer/query counts, cluster info (nodeId, region, isolation). |
| `GET` | `/status/health` | Health check. Returns 503 if cluster is in isolated/split-brain state. |
| `GET` | `/health/ready` | Kubernetes readiness probe. Checks stack initialization. |
| `GET` | `/health/live` | Kubernetes liveness probe. Returns uptime. |

### Cluster Operations

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/cluster/status` | Cluster membership: node list with IDs, hosts, ports, RTT, CPU load, region, last heartbeat. |
| `POST` | `/cluster/query` | Federated cross-cluster query. Forwards search to peer clusters and aggregates results. |
| `POST` | `/cluster/heartbeat` | Receive heartbeat from a cluster peer node. |
| `GET` | `/cluster/results/:id` | Get federated query results from a remote node. |

### Admin Operations

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/admin/peers/:id/ban` | Ban a peer by ID. |
| `DELETE` | `/admin/peers/:id/ban` | Unban a peer. |
| `GET` | `/admin/peers/banned` | List banned peers. |
| `POST` | `/admin/config/reload` | Reload configuration file at runtime. |
| `GET` | `/admin/logs/level` | Get current log level. |
| `PUT` | `/admin/logs/level` | Set log level: Silent, Error, Info, Debug. |
| `POST` | `/admin/keys/rotate` | Rotate API key. Body: `{grace_period_seconds?}` (default 3600). Old key valid during grace period. |
| `POST` | `/admin/ipfilter/allow` | Add IP/CIDR to allowlist. |
| `POST` | `/admin/ipfilter/block` | Add IP/CIDR to blocklist. |
| `GET` | `/admin/ipfilter` | List current IP filter rules. |
| `DELETE` | `/admin/ipfilter/:ip` | Remove an IP filter rule. |

### Authentication

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/auth/enroll-device` | Enroll a device with public key. Body: `{publicKey, deviceName?, biometricType?}`. |
| `GET` | `/auth/challenge` | Get a time-limited authentication challenge nonce. |
| `POST` | `/auth/verify` | Verify challenge-response signature. Body: `{challengeId, signature, publicKey}`. |

### Observability

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/metrics` | Prometheus exposition format: histograms, counters, gauges. |
| `GET` | `/api` | API documentation with all registered routes. |

### VFS (when FUSE enabled)

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/vfs/stats` | Access statistics across all resources. |
| `GET` | `/vfs/stats/popular` | Most accessed resources. |
| `GET` | `/vfs/stats/recent` | Most recently accessed resources. |
| `GET` | `/vfs/stats/peer/:id` | Per-peer access statistics. |
| `GET` | `/vfs/status` | VFS mount status. |

Response compression is automatic — include `Accept-Encoding: gzip` for bandwidth savings on large result sets.

---

## CLI Reference

### Shell Setup

```sh
# Bash — add to ~/.bashrc
eval "$(AlpineCmdIf --completions bash)"

# Zsh — add to ~/.zshrc
eval "$(AlpineCmdIf --completions zsh)"

# Or source the universal setup script:
source AlpineCmdIf/completions/setup.sh
```

This provides the `alpc` alias, tab completion, and quick-action shortcuts.

### Commands

#### Query Commands

| Command | Description | Key Args |
|---------|-------------|----------|
| `beginQuery` | Start a distributed query | `--queryString`, `--numHits?`, `--moduleId?` |
| `getQueryStatus` | Check query progress | `--queryId` |
| `pauseQuery` | Pause a running query | `--queryId` |
| `resumeQuery` | Resume a paused query | `--queryId` |
| `cancelQuery` | Cancel a query | `--queryId` |
| `getQueryResults` | Get query results | `--queryId` |

#### Peer Commands

| Command | Description | Key Args |
|---------|-------------|----------|
| `addDtcpPeer` | Add a peer by IP and port | `--ipAddress`, `--port` |
| `getDtcpPeerId` | Get peer ID for an address | `--ipAddress`, `--port` |
| `getDtcpPeerStatus` | Get peer status details | `--peerId` |
| `activateDtcpPeer` | Activate a peer connection | `--peerId` |
| `deactivateDtcpPeer` | Deactivate a peer | `--peerId` |
| `pingDtcpPeer` | Ping a peer | `--peerId` |
| `getExtendedPeerList` | List all peers with details | — |

#### Network Filtering

| Command | Description | Key Args |
|---------|-------------|----------|
| `excludeHost` | Block a host | `--ipAddress` |
| `excludeSubnet` | Block a subnet | `--subnetIpAddress`, `--subnetMask` |
| `allowHost` | Unblock a host | `--ipAddress` |
| `allowSubnet` | Unblock a subnet | `--subnetIpAddress` |
| `listExcludedHosts` | List blocked hosts | — |
| `listExcludedSubnets` | List blocked subnets | — |

#### Group & Module Commands

| Command | Description | Key Args |
|---------|-------------|----------|
| `getUserGroupList` | List all user groups | — |
| `createUserGroup` / `destroyUserGroup` | Create/delete a group | `--groupName` |
| `addPeerToGroup` / `removePeerFromGroup` | Manage group membership | `--peerId`, `--groupName` |
| `registerModule` | Register a plugin module | `--moduleName`, `--modulePath` |
| `loadModule` / `unloadModule` | Load/unload a module | `--moduleId` |
| `listActiveModules` / `listAllModules` | List modules | — |
| `getModuleInfo` | Get module details | `--moduleId` |

#### System

| Command | Description |
|---------|-------------|
| `getStatus` | Server status and version |
| `interactive` | Launch REPL with history and tab completion |

### Global Flags

| Flag | Description |
|------|-------------|
| `--json` | JSON output |
| `--quiet` | Suppress logging |
| `--format json\|table\|csv\|yaml` | Output format |
| `--color` | Force colored output |
| `--verbose` | Verbose mode |
| `--completions bash\|zsh` | Generate shell completions |

### Quick-Action Aliases

| Alias | Equivalent |
|-------|-----------|
| `alpc-status` | `AlpineCmdIf --command getStatus --json --quiet` |
| `alpc-peers` | `AlpineCmdIf --command getExtendedPeerList --json --quiet` |
| `alpc-query` | `AlpineCmdIf --command beginQuery --json --quiet` |
| `alpc-results` | `AlpineCmdIf --command getQueryResults --json --quiet` |
| `alpc-modules` | `AlpineCmdIf --command listActiveModules --json --quiet` |
| `alpc-interactive` | `AlpineCmdIf --command interactive` |

Table-formatted variants append `-t` (e.g., `alpc-peers-t`).

### Helper Functions

```sh
alpc-search "search terms" [max_hits]     # Start query and return results
alpc-peer-add <ip> <port>                 # Add a peer
alpc-peer-info <peer_id>                  # Peer details (table format)
alpc-query-watch <query_id> [interval]    # Poll until query completes
alpc-block <ip>                           # Block an IP via filter
alpc-unblock <ip>                         # Unblock an IP
```

---

## Protocol Stack

### Query Lifecycle

```
  Originator                    Peer B                    Peer C
      │                           │                         │
      │──── queryDiscover ───────►│                         │
      │──── queryDiscover ────────┼────────────────────────►│
      │                           │                         │
      │◄──── queryOffer ──────────│  (hitCount=12)          │
      │◄──── queryOffer ──────────┼─────────────────────────│  (hitCount=5)
      │                           │                         │
      │  [RatingEngine selects    │                         │
      │   best peers by score]    │                         │
      │                           │                         │
      │──── queryRequest ────────►│                         │
      │──── queryRequest ─────────┼────────────────────────►│
      │                           │                         │
      │◄──── queryReply ──────────│  (resource descriptors) │
      │◄──── queryReply ──────────┼─────────────────────────│
      │                           │                         │
      ▼ Aggregate & deliver       │                         │
```

### Protocol Versioning

Packets use a zero-marker format for backward compatibility:

- **Versioned packet**: `[0x0000][version uint16][packetType uint16][payload...]`
- **Legacy packet**: `[packetType uint16][payload...]`

New nodes detect the `0x0000` marker and read the version. Old nodes see `packetType=0` (none) and ignore the packet gracefully.

### Query Priority

Queries carry a priority byte (0-255, default 128). The query manager processes higher-priority queries first using priority-sorted snapshots in `processTimedEvents()`. Priority propagates across every hop in a multi-node query chain.

### Peer Quality Metrics

The `AlpineRatingEngine` tracks per-peer quality scores using:
- Exponential moving average of success/failure rates
- Configurable score decay over time
- Gossip-based score merging across cluster nodes
- `CircuitBreaker` with configurable thresholds:
  - **Closed** → **Open**: after N consecutive failures (default 5)
  - **Open** → **HalfOpen**: after timeout (default 30 seconds)
  - **HalfOpen** → **Closed**: on successful probe
  - **HalfOpen** → **Open**: on failed probe

### Peer Exchange

```
peerListRequest ──► peerListOffer ──► peerListGet ──► peerListData
```

### Proxy Routing

```
proxyRequest ──► proxyAccepted / proxyHalt
```

---

## Configuration

### AlpineRestBridge Configuration

All settings follow the precedence: config file > command-line args > environment variables.

#### Network & Ports

| Config Name | Env Var | Default | Description |
|-------------|---------|---------|-------------|
| IP Address | `IP_ADDRESS` | — | Bind address (required) |
| Port | `PORT` | 9000 | Protocol port |
| REST Port | `REST_PORT` | 8080 | HTTP API port |
| REST Bind Address | `REST_BIND_ADDRESS` | `0.0.0.0` | HTTP bind address |

#### HTTP Server Tuning

| Config Name | Env Var | Default | Range | Description |
|-------------|---------|---------|-------|-------------|
| HTTP Min Threads | `HTTP_MIN_THREADS` | 4 | 1-256 | Minimum worker threads |
| HTTP Max Threads | `HTTP_MAX_THREADS` | 32 | 1-1024 | Maximum worker threads |
| HTTP Max Connections | `HTTP_MAX_CONNECTIONS` | 512 | — | Total concurrent connections |
| HTTP Max Connections Per IP | `HTTP_MAX_CONNECTIONS_PER_IP` | 16 | — | Per-IP connection limit |
| HTTP Idle Timeout | `HTTP_IDLE_TIMEOUT_SECONDS` | 60 | 1-3600 | Idle connection timeout |
| HTTP Keep-Alive Max | `HTTP_KEEPALIVE_MAX_REQUESTS` | 100 | 1-10000 | Max requests per connection |
| HTTP Write Timeout | `HTTP_WRITE_TIMEOUT_SECONDS` | 10 | 1-300 | Write timeout for slow clients |
| Shutdown Drain | `SHUTDOWN_DRAIN_SECONDS` | 5 | 1-60 | Graceful shutdown timeout |

#### Authentication & Security

| Config Name | Env Var | Default | Description |
|-------------|---------|---------|-------------|
| API Key | `ALPINE_API_KEY` | auto-generated | 64-char hex API key |
| CORS Origin | `CORS_ORIGIN` | — | Allowed CORS origin (`*` or specific domain) |

#### Discovery & Cluster

| Config Name | Env Var | Default | Description |
|-------------|---------|---------|-------------|
| Beacon Enabled | `BEACON_ENABLED` | `true` | UDP discovery beacon |
| Beacon Port | `BEACON_PORT` | 8089 | Beacon listen port |
| Broadcast Enabled | `BROADCAST_ENABLED` | `true` | Broadcast query handler |
| Broadcast Port | `BROADCAST_PORT` | 8090 | Broadcast query port |
| Region | `ALPINE_REGION` | `default` | Cluster region identifier |
| WiFi Multicast Group | `WIFI_MULTICAST_GROUP` | — | Multicast group address |
| WiFi Multicast Port | `WIFI_MULTICAST_PORT` | — | Multicast port |

#### Optional Services

| Config Name | Env Var | Default | Description |
|-------------|---------|---------|-------------|
| Log Level | `LOG_LEVEL` | `Info` | Silent, Error, Info, Debug |
| Tor Enabled | `TOR_ENABLED` | `false` | Tor hidden service |
| DLNA Enabled | `DLNA_ENABLED` | `false` | DLNA media server |
| FUSE Enabled | `FUSE_ENABLED` | `false` | Virtual filesystem |
| FUSE Mount Point | `FUSE_MOUNT_POINT` | `/tmp/alpine` | VFS mount path |
| FUSE Cache TTL | `FUSE_CACHE_TTL` | 60 | Query cache TTL (seconds) |
| Tracing Enabled | `TRACING_ENABLED` | `false` | OpenTelemetry tracing |
| OTLP Endpoint | `OTLP_ENDPOINT` | — | OpenTelemetry collector URL |

#### Webhooks

| Config Name | Env Var | Default | Description |
|-------------|---------|---------|-------------|
| Webhook Secret | `WEBHOOK_SECRET` | — | HMAC secret for callback signatures |
| Webhook Max Retries | `WEBHOOK_MAX_RETRIES` | 3 | Delivery retry count |
| Webhook Timeout | `WEBHOOK_TIMEOUT_SECONDS` | 10 | Delivery timeout |

---

## Security

### Authentication

- **API Key** — 64-character hex keys (32 bytes random). Stored with `SecureString` for secure memory erasure. Constant-time comparison prevents timing attacks.
- **Key Rotation** — `POST /admin/keys/rotate` with configurable grace period. Old keys remain valid during transition for zero-downtime rotation.
- **JWT** — Optional JWT validation with HS256 (when TLS enabled). Scope-based authorization.
- **Device Enrollment** — Challenge-response authentication with public key cryptography and time-limited nonces.

### Authorization

- **RBAC** — Role-based access control via JSON policy file. Permissions use `resource:action` format (e.g., `query:start`, `admin:ban`). Wildcard `"*"` grants full access. When RBAC is disabled, all authenticated requests are allowed.

### Network Security

- **TLS** — Hardened cipher suites: TLS 1.3 AEAD-only (AES-256-GCM, ChaCha20-Poly1305), TLS 1.2 ECDHE/DHE with AEAD. Weak ciphers explicitly disabled.
- **IP Filtering** — CIDR-based allowlist/blocklist with blocklist precedence. Managed via REST endpoints or config files.
- **P2P Packet Authentication** — HMAC-SHA256 per-peer authentication with per-peer shared secrets stored via `SecureString`.
- **Rate Limiting** — Sharded token bucket per IP (16 shards) with configurable rate and burst. Returns 429 when exceeded.

### Protocol Hardening

- **Packet bounds checking** — All `bufferSize - readLength` operations guarded against unsigned integer underflow.
- **Resource limits** — `MAX_HITS = 10,000` per query offer, `MAX_REPLY_SET_SIZE = 10,000` per reply.
- **String length caps** — `MAX_STRING_LEN = 65,536` bytes enforced in packet parsing to prevent unbounded allocation from malicious packets.
- **Partial write handling** — TCP send operations loop until all bytes are transmitted, with EINTR retry.
- **Body validation** — HTTP requests with Content-Length are validated for completeness; truncated bodies are rejected with 400.

### Data Protection

- **Secure Memory** — `SecureString` class erases secrets from memory on destruction using platform-specific secure zeroing (`explicit_bzero`, `SecureZeroMemory`).
- **Log Sanitization** — User-controlled strings are sanitized via `StringUtils::sanitizeForLog()` before logging to prevent log injection.
- **Content Integrity** — Optional `Content-SHA256` header validation on request bodies.
- **Config File Permissions** — Warns on startup if config files have group/other read permissions.
- **Audit Logging** — Structured JSON-lines audit trail for admin actions, auth events, and query operations via `AuditLog`.

### CORS

When CORS origin is `*` (wildcard), only safe methods (`GET`, `POST`, `OPTIONS`) are allowed. `DELETE` requires a specific origin with `Access-Control-Allow-Credentials: true`.

---

## Observability

### Prometheus Metrics

`GET /metrics` returns metrics in Prometheus exposition format:

```
# TYPE http_request_duration_seconds histogram
http_request_duration_seconds_bucket{le="0.005"} 24
http_request_duration_seconds_bucket{le="0.01"} 33
...
http_request_duration_seconds_bucket{le="+Inf"} 144
http_request_duration_seconds_sum 53.2
http_request_duration_seconds_count 144

# TYPE http_requests_total counter
http_requests_total{method="GET"} 1205
http_requests_total{method="POST"} 342

# TYPE queries_total counter
queries_total{status="started"} 89
queries_total{status="completed"} 85
queries_total{status="failed"} 4
```

Pre-registered counters: `http_requests_total` (by method), `queries_total` (by status), `peers_total` (connected/disconnected), `websocket_sessions_total`, `rate_limited_total`.

Histogram buckets at: 5ms, 10ms, 25ms, 50ms, 100ms, 250ms, 500ms, 1s, 2.5s, 5s, 10s.

### OpenTelemetry Tracing

When enabled (`ALPINE_ENABLE_TRACING=ON`), spans are emitted for HTTP requests, query lifecycle events, and peer communication. Trace context propagates across P2P queries using W3C `traceparent` headers embedded in query packets. Configure the OTLP collector endpoint via `OTLP_ENDPOINT`.

### Structured Logging

`Log::Error/Info/Debug` with `std::format` syntax: `Log::Info("processed {} queries in {}ms", count, elapsed)`. Supports correlation IDs via `X-Request-ID` headers.

---

## Deployment

### Docker Compose

```sh
# 3-node cluster
docker-compose -f docker/docker-compose.yml up

# 5-node cluster (with benchmark nodes)
docker-compose -f docker/docker-compose.yml --profile bench up
```

Network layout:

| Node | IP | REST Port | Protocol Port |
|------|----|-----------|---------------|
| node1 | 172.28.1.1 | localhost:8081 | 9000 |
| node2 | 172.28.1.2 | localhost:8082 | 9000 |
| node3 | 172.28.1.3 | localhost:8083 | 9000 |
| node4 (bench) | 172.28.1.4 | localhost:8084 | 9000 |
| node5 (bench) | 172.28.1.5 | localhost:8085 | 9000 |

All nodes share a bridge network (`172.28.0.0/16`) with UDP beacon discovery on port 8089 and WiFi multicast on `224.0.1.75:41750`. Health checks poll `/status` every 5 seconds. Node2 and node3 wait for node1 to become healthy before starting.

### Kubernetes (Helm)

```sh
helm install alpine k8s/helm/alpine/ \
  --set replicaCount=3 \
  --set config.logLevel=Info \
  --set autoscaling.enabled=true
```

Features:

- **HPA** — Auto-scales 2-10 replicas targeting 70% CPU utilization
- **PDB** — Pod disruption budget (maxUnavailable: 1) for zero-downtime updates
- **Persistent Volumes** — Optional PVC for SQLite data and rating engine state
- **Health Probes** — Readiness (`/health/ready`, 5s initial delay) and liveness (`/health/live`, 10s initial delay)
- **Network Policy** — Enabled by default to restrict pod traffic
- **Resource Limits** — Default 100m/128Mi request, 500m/512Mi limit per pod

Exposed ports: 8080 (REST), 8089 (beacon), 8090 (broadcast), 9000 (protocol).

---

## Building

### Standard Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Debug / Profile / Sanitizer Builds

```sh
# Debug
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Profile (gprof)
cmake -B build -DCMAKE_BUILD_TYPE=Profile

# Sanitizers
cmake -B build -DALPINE_SANITIZER=address,undefined
```

### Optional Features

| CMake Flag | Default | Description |
|-----------|---------|-------------|
| `ALPINE_BUILD_TESTS` | `ON` | Unit and integration tests (Catch2) |
| `ALPINE_ENABLE_TLS` | `OFF` | TLS/DTLS encryption (OpenSSL) |
| `ALPINE_ENABLE_PERSISTENCE` | `OFF` | SQLite persistence layer |
| `ALPINE_ENABLE_TRACING` | `OFF` | OpenTelemetry distributed tracing |
| `ALPINE_ENABLE_UPNP` | `OFF` | UPnP IGD port mapping |
| `ALPINE_ENABLE_FUSE` | `OFF` | FUSE virtual filesystem |
| `ALPINE_ENABLE_CORBA` | `OFF` | CORBA remote management (ACE/TAO) |
| `ALPINE_ENABLE_COVERAGE` | `OFF` | Code coverage instrumentation |
| `ALPINE_BUILD_BENCHMARKS` | `OFF` | Google Benchmark targets |
| `ALPINE_BUILD_FUZZERS` | `OFF` | libFuzzer targets |
| `ALPINE_USE_SYSTEM_DEPS` | `OFF` | Use system packages instead of FetchContent |
| `ALPINE_SANITIZER` | — | Comma-separated sanitizers (address, undefined, thread, memory) |

System dependencies can alternatively be provided via Conan (`conanfile.py`) with `ALPINE_USE_SYSTEM_DEPS=ON`.

### Dependencies (fetched automatically)

| Library | Version | Purpose |
|---------|---------|---------|
| nlohmann/json | 3.11.3 | JSON serialization |
| ASIO | 1.30.2 | Async networking (standalone, no Boost) |
| spdlog | 1.14.1 | Logging backend |
| linenoise | latest | CLI line editing |
| Catch2 | 3.5.2 | Test framework |
| jwt-cpp | 0.7.0 | JWT handling (when TLS enabled) |
| miniupnpc | 2.2.7 | UPnP client (when UPnP enabled) |
| sqlite3 | 3.45.0 | Persistence (when enabled) |
| opentelemetry-cpp | 1.14.2 | Tracing (when enabled) |
| benchmark | 1.8.3 | Benchmarks (when enabled) |

---

## Testing

### Run Tests

```sh
cd build && ctest --output-on-failure --timeout 120
```

### Unit Tests (Catch2)

Security-critical: `test_api_key_auth`, `test_ip_filter`, `test_circuit_breaker`, `test_rbac_policy`, `test_packet_auth`, `test_secure_string`

Core functionality: `test_http_request`, `test_http_response`, `test_http_router`, `test_configuration`, `test_error`, `test_string_utils`, `test_json_writer`, `test_datablock`, `test_readwritesem`, `test_compression`, `test_webhook_dispatcher`, `test_rate_limiter`, `test_query_cache`, `test_audit_log`, `test_access_tracker`

### Integration Tests

`test_rest_query_lifecycle`, `test_rest_peer_endpoints`, `test_rest_auth_flow`, `test_rest_rate_limiting`

### Load Testing

```sh
./build/bin/rest_load_test --host localhost --port 8080 \
  --connections 10 --requests-per-conn 100 --path /status
```

Reports requests/sec and p50/p95/p99 latency.

### Benchmarks

```sh
cmake -B build -DALPINE_BUILD_BENCHMARKS=ON
./build/bin/bench_datablock
./build/bin/bench_http_parse
```

### Code Coverage

```sh
cmake -B build -DALPINE_ENABLE_COVERAGE=ON -DALPINE_BUILD_TESTS=ON
cmake --build build && cd build && ctest
lcov --capture --directory . --output-file coverage.info
```

### Fuzzing

```sh
cmake -B build -DALPINE_BUILD_FUZZERS=ON
./build/bin/fuzz_http_parse
./build/bin/fuzz_http_router
```

---

## Module Plugin System

Alpine supports dynamically loaded modules (`.so`/`.dll`) that extend query, peer discovery, and proxy capabilities:

```sh
# Register and load a module
alpc --command registerModule --moduleName myPlugin --modulePath ./libMyPlugin.so
alpc --command loadModule --moduleId 1

# List active modules
alpc --command listActiveModules
```

Modules implement the `AlpineModuleInterface` and are managed by `AlpineModuleMgr` with registration, loading/unloading, and dependency tracking.

---

## FUSE Virtual Filesystem

When compiled with `ALPINE_ENABLE_FUSE=ON`, Alpine mounts a virtual filesystem exposing queries, peers, and statistics as files:

```
/alpine/
  queries/
    <query_id>/
      status    <- query status JSON
      results   <- query results JSON
  peers/
    <peer_id>   <- peer info JSON
  stats/
    access      <- access statistics
    popular     <- popular resources
    recent      <- recent queries
```

The `QueryCache` provides an LRU cache with configurable TTL for query results. The `AccessTracker` records per-resource, per-peer, and per-query-term access statistics, serializable as JSON or plain text.

REST endpoints for stats: `GET /vfs/stats`, `/vfs/stats/popular`, `/vfs/stats/recent`, `/vfs/stats/peer/:id`.

---

## Platform Support

| Platform | Compiler | Status |
|----------|----------|--------|
| Linux (Ubuntu 24.04) | GCC-14, Clang-18 | CI tested |
| macOS | Apple Clang | CI tested |
| Windows | MinGW-w64 | Supported |
| Docker | amd64, arm64 | Multi-arch |
| BSD (FreeBSD, DragonFly) | Clang | Supported |

CI pipeline: build + test (3 compiler configs), sanitizers (ASan + UBSan), clang-format-18, clang-tidy-18, code coverage, Docker build.

---

## License

Alpine is released under the MIT License. See [LICENCE.txt](LICENCE.txt) for details.
