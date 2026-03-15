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

```
              ┌──────────┐
              │  Client   │
              │ (REST/CLI)│
              └─────┬─────┘
                    │ POST /query
                    ▼
              ┌──────────┐        queryDiscover
              │  Node A   │─────────────────────────┐
              │ (Bridge)  │◀── queryOffer ──┐        │
              └─────┬─────┘                 │        │
                    │                 ┌─────┴────┐   │
                    │                 │  Node B   │   │
                    │                 └──────────┘   │
                    │                          ┌─────┴────┐
                    │                          │  Node C   │
                    │                          └──────────┘
                    │ queryRequest ──▶ B, C
                    │◀── queryReply ── B, C
                    │
                    ▼
              ┌──────────┐
              │ Aggregated│
              │  Results  │
              └──────────┘
```

---

## Use Cases

### Media Sharing on a Home Network

A family has media files spread across a NAS, a laptop, and a Raspberry Pi. Each device runs an Alpine node. When someone searches for "vacation photos," the query is broadcast to all devices simultaneously. Each node checks its local content index and returns matching files. The results are merged and presented in a single list — no central media server required. DLNA integration means standard media players can browse the shared library directly.

### Distributed Enterprise Search

An organization has data spread across departments — engineering docs, sales reports, HR records — each on a separate server. Alpine nodes on each server register their resources. When an authorized user queries from any endpoint, the search fans out to every department node. Results are filtered by the user's RBAC role (a `query`-role key can search but not access admin endpoints). The rating engine learns which servers respond fastest, optimizing future query routing.

### IoT and Edge Service Discovery

In an industrial IoT deployment, edge devices need to discover available services (sensor data feeds, actuator controls, firmware update servers) without a central registry that becomes a single point of failure. Alpine nodes on each device broadcast their capabilities via WiFi multicast. New devices are discovered automatically. If a device goes offline, the circuit breaker marks it degraded and queries route around it.

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
curl http://localhost:8081/status

# Start a query
curl -X POST http://localhost:8081/query \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <api-key>" \
  -d '{"queryString": "example search"}'

# Get results
curl http://localhost:8081/query/1/results
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
┌──────────────────────────────────────────────────────────┐
│                      Interfaces                           │
│    AlpineStackInterface    │    DtcpStackInterface        │
├──────────────────────────────────────────────────────────┤
│                Application Core (ApplCore)                │
│    Lifecycle · Signals · Configuration · Logging          │
├──────────────────────────────────────────────────────────┤
│              Protocol Layer (AlpineProtocol)              │
│   QueryMgr · PeerMgr · GroupMgr · ModuleMgr              │
│   RatingEngine · CircuitBreaker · PacketAuth · IpFilter   │
├──────────────────────────────────────────────────────────┤
│             Transport Layer (AlpineTransport)             │
│   UDP Unicast · Multicast · Broadcast · WiFi Direct       │
│   DTCP Reliable · DtlsWrapper · SendQueue                 │
├──────────────────────────────────────────────────────────┤
│                    Base Libraries                         │
│   AppUtils · SysUtils · ThreadUtils · NetUtils            │
│   ConfigUtils · VfsUtils · Tracing                        │
└──────────────────────────────────────────────────────────┘
```

### Key Binaries

| Binary | Purpose |
|--------|---------|
| `AlpineServer` | Standalone server daemon with JSON-RPC interface |
| `AlpineCmdIf` | Interactive CLI client (`alpc` alias) |
| `AlpineRestBridge` | Full-featured REST API server with DLNA, mDNS, SSDP, Tor, cluster coordination |

### Design Patterns

- **Static Facade** — Major classes (`AlpineStackInterface`, `ApplCore`, `Configuration`, `Log`) are pure static with thread-safe static members.
- **RAII Guards** — `ConnectionGuard` for HTTP connections, `ReadLock`/`WriteLock` for shared state.
- **Sharded Locking** — IP rate limiting, connection tracking, and dedup indices use 8-16 shard arrays for low contention.
- **Dynamic Thread Pool** — HTTP server scales from 4 to 32 worker threads based on load.
- **Keep-Alive** — HTTP connections are reused for up to 100 requests (configurable).
- **Cooperative Cancellation** — Threads use `std::jthread` with `std::stop_token` for clean lifecycle management.

---

## REST API Reference

All endpoints require API key authentication via `Authorization: Bearer <key>` header unless noted.

### Query Operations

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/query` | Start a distributed query. Body: `{queryString, groupName?, priority?, numHits?, callbackUrl?}`. Returns 202 with `{queryId}`. |
| `GET` | `/query/:id` | Get query status. Returns `{queryId, inProgress, totalPeers, peersQueried, numPeerResponses, totalHits}`. |
| `GET` | `/query/:id/results` | Get paginated results. Params: `limit` (max 1000), `offset`. Returns `{data[], total, hasMore}`. |
| `GET` | `/query/:id/stream` | Server-Sent Events stream. Events: `result`, `progress`, `complete`. |
| `DELETE` | `/query/:id` | Cancel a running query. |

**Query priority**: 0 (lowest) to 255 (highest), default 128. Higher-priority queries are processed first.

**Webhooks**: Include `callbackUrl` in the POST body. On completion, Alpine POSTs results to the URL with an `X-Alpine-Signature` HMAC header.

### Peer Operations

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/peers` | List all peers. Params: `limit`, `offset`, `status` (active/inactive), `minScore`. |
| `GET` | `/peers/:id` | Get peer details: IP, port, bandwidth, timing stats. |

### Status & Health

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/status` | Server status, version, uptime, peer/query counts, cluster info. |
| `GET` | `/status/health` | Health check. 503 if cluster isolated. |
| `GET` | `/health/ready` | Kubernetes readiness probe. |
| `GET` | `/health/live` | Kubernetes liveness probe. |

### Admin Operations

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/admin/peers/:id/ban` | Ban a peer. |
| `DELETE` | `/admin/peers/:id/ban` | Unban a peer. |
| `GET` | `/admin/peers/banned` | List banned peers. |
| `POST` | `/admin/config/reload` | Reload configuration file. |
| `GET` | `/admin/logs/level` | Get current log level. |
| `PUT` | `/admin/logs/level` | Set log level (Silent/Error/Info/Debug). |
| `POST` | `/admin/keys/rotate` | Rotate API key. Body: `{grace_period_seconds?}` (default 3600). |

### Authentication

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/auth/enroll-device` | Enroll biometric device. Body: `{publicKey, deviceName?, biometricType?}`. |
| `GET` | `/auth/challenge` | Get authentication challenge. |
| `POST` | `/auth/verify` | Verify challenge signature. |

### Cluster

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/cluster/status` | Cluster membership and node status. |
| `POST` | `/cluster/query` | Federated cross-cluster query. |
| `POST` | `/cluster/heartbeat` | Receive heartbeat from cluster peer. |
| `GET` | `/cluster/results/:id` | Get federated query results. |

### Other

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/metrics` | Prometheus-format metrics (histograms, counters, gauges). |
| `GET` | `/api` | API documentation (route listing). |
| `GET` | `/vfs/stats` | VFS access statistics (when FUSE enabled). |

Response compression is automatic — include `Accept-Encoding: gzip` for 5-10x bandwidth savings on large results.

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

#### Query Commands

| Command | Description | Key Args |
|---------|-------------|----------|
| `beginQuery` | Start a distributed query | `--queryString`, `--numHits?`, `--moduleId?` |
| `getQueryStatus` | Check query progress | `--queryId` |
| `pauseQuery` | Pause a running query | `--queryId` |
| `resumeQuery` | Resume a paused query | `--queryId` |
| `cancelQuery` | Cancel a query | `--queryId` |
| `getQueryResults` | Get query results | `--queryId` |

#### Group Commands

| Command | Description | Key Args |
|---------|-------------|----------|
| `getUserGroupList` | List all user groups | — |
| `createUserGroup` | Create a group | `--groupName` |
| `destroyUserGroup` | Delete a group | `--groupName` |
| `addPeerToGroup` | Add peer to group | `--peerId`, `--groupName` |
| `removePeerFromGroup` | Remove peer from group | `--peerId`, `--groupName` |

#### Module Commands

| Command | Description | Key Args |
|---------|-------------|----------|
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
alpc-search "search terms" [max_hits]     # Start query
alpc-peer-add <ip> <port>                 # Add peer
alpc-peer-info <peer_id>                  # Peer details (table)
alpc-query-watch <query_id> [interval]    # Poll until complete
alpc-block <ip>                           # Block an IP
alpc-unblock <ip>                         # Unblock an IP
```

---

## Protocol Stack

### Query Lifecycle

```
queryDiscover ──▶ queryOffer ──▶ queryRequest ──▶ queryReply
```

1. **Discover** — Originator broadcasts query metadata to peers
2. **Offer** — Peers with matching resources respond with hit count
3. **Request** — Originator requests full results from offering peers
4. **Reply** — Peers send resource descriptors back

### Protocol Versioning

Packets use a zero-marker format for backward compatibility:

- **Versioned packet**: `[0x0000][version uint16][packetType uint16][payload...]`
- **Legacy packet**: `[packetType uint16][payload...]`

New nodes detect the `0x0000` marker and read the version. Old nodes see `packetType=0` (none) and ignore the packet gracefully.

### Query Priority

Queries carry a priority byte (0-255, default 128). The query manager processes higher-priority queries first using priority-sorted snapshots in `processTimedEvents()`.

### Peer Quality Metrics

The `AlpineRatingEngine` tracks per-peer quality scores using:
- Exponential moving average of success/failure rates
- Configurable score decay over time
- Gossip-based score merging across cluster nodes
- `CircuitBreaker` with open/half-open/closed states

### Peer Exchange

```
peerListRequest ──▶ peerListOffer ──▶ peerListGet ──▶ peerListData
```

### Proxy Routing

```
proxyRequest ──▶ proxyAccepted / proxyHalt
```

---

## Configuration

### AlpineRestBridge Configuration

| Config Name | Env Var | Default | Description |
|-------------|---------|---------|-------------|
| IP Address | `IP_ADDRESS` | — | Bind address (required) |
| Port | `PORT` | — | Protocol port (required) |
| REST Port | `REST_PORT` | — | HTTP API port (required) |
| REST Bind Address | `REST_BIND_ADDRESS` | `0.0.0.0` | HTTP bind address |
| API Key | `ALPINE_API_KEY` | auto-generated | 64-char hex API key |
| CORS Origin | `CORS_ORIGIN` | — | Allowed CORS origin (`*` or specific domain) |
| Log Level | `LOG_LEVEL` | `Info` | Silent, Error, Info, Debug |
| HTTP Min Threads | `HTTP_MIN_THREADS` | `4` | Min worker threads (1-256) |
| HTTP Max Threads | `HTTP_MAX_THREADS` | `32` | Max worker threads (1-1024) |
| HTTP Max Connections | `HTTP_MAX_CONNECTIONS` | `512` | Max concurrent connections |
| HTTP Max Connections Per IP | `HTTP_MAX_CONNECTIONS_PER_IP` | `16` | Per-IP connection limit |
| HTTP Idle Timeout | `HTTP_IDLE_TIMEOUT_SECONDS` | `60` | Idle connection timeout (1-3600s) |
| HTTP Keep-Alive Max | `HTTP_KEEPALIVE_MAX_REQUESTS` | `100` | Max requests per connection (1-10000) |
| HTTP Write Timeout | `HTTP_WRITE_TIMEOUT_SECONDS` | `10` | Write timeout for slow clients (1-300s) |
| Shutdown Drain | `SHUTDOWN_DRAIN_SECONDS` | `5` | Graceful shutdown timeout (1-60s) |
| Beacon Enabled | `BEACON_ENABLED` | `true` | UDP discovery beacon |
| Beacon Port | `BEACON_PORT` | `8089` | Beacon listen port |
| Broadcast Enabled | `BROADCAST_ENABLED` | `true` | Broadcast query handler |
| Broadcast Port | `BROADCAST_PORT` | `8090` | Broadcast query port |
| Tor Enabled | `TOR_ENABLED` | `false` | Tor hidden service |
| DLNA Enabled | `DLNA_ENABLED` | `false` | DLNA media server |
| FUSE Enabled | `FUSE_ENABLED` | `false` | Virtual filesystem |
| FUSE Mount Point | `FUSE_MOUNT_POINT` | `/tmp/alpine` | VFS mount path |
| FUSE Cache TTL | `FUSE_CACHE_TTL` | `60` | Query cache TTL (seconds) |
| Tracing Enabled | `TRACING_ENABLED` | `false` | OpenTelemetry tracing |
| OTLP Endpoint | `OTLP_ENDPOINT` | — | OpenTelemetry collector URL |
| Webhook Secret | `WEBHOOK_SECRET` | — | HMAC secret for webhook signatures |
| Webhook Max Retries | `WEBHOOK_MAX_RETRIES` | `3` | Webhook delivery retries |
| Webhook Timeout | `WEBHOOK_TIMEOUT_SECONDS` | `10` | Webhook delivery timeout |
| WiFi Multicast Group | `WIFI_MULTICAST_GROUP` | — | Multicast group address |
| WiFi Multicast Port | `WIFI_MULTICAST_PORT` | — | Multicast port |

---

## Security

### Authentication

- **API Key** — 64-character hex keys (32 bytes random). Stored with `SecureString` for secure memory erasure. Constant-time comparison prevents timing attacks.
- **Key Rotation** — `POST /admin/keys/rotate` with configurable grace period. Old keys remain valid during transition.
- **JWT** — Optional JWT validation with HS256 (when TLS enabled). Scope-based authorization.
- **Device Enrollment** — Challenge-response authentication with public key cryptography.

### Authorization

- **RBAC** — Role-based access control via JSON policy file. Permissions use `resource:action` format (e.g., `query:start`, `admin:ban`). When RBAC is disabled, all authenticated requests are allowed.

### Network Security

- **TLS** — Hardened cipher suites: TLS 1.3 AEAD-only (AES-GCM, ChaCha20-Poly1305), TLS 1.2 ECDHE/DHE with AEAD. Weak ciphers explicitly disabled.
- **IP Filtering** — CIDR-based allowlist/blocklist. Managed via REST endpoints or config files. Blocklist takes precedence.
- **P2P Packet Authentication** — HMAC-SHA256 per-peer authentication. Secrets stored using `SecureString`.
- **Rate Limiting** — Sharded token bucket per IP with configurable rate and burst.

### Data Protection

- **Secure Memory** — `SecureString` class erases secrets from memory on destruction using platform-specific secure zeroing (`explicit_bzero`, `SecureZeroMemory`).
- **Log Sanitization** — User-controlled strings are sanitized before logging to prevent log injection.
- **Content Integrity** — Optional `Content-SHA256` header validation on request bodies.
- **Config File Permissions** — Warns on startup if config files have group/other read permissions.
- **Audit Logging** — Structured JSON-lines audit trail for admin actions, auth events, and query operations.

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
```

Pre-registered counters: `HttpRequestGet`, `HttpRequestPost`, `QueryStarted`, `QueryCompleted`, `QueryFailed`, `PeerConnected`, `PeerDisconnected`, `RateLimited`, `WebSocketOpened`, `WebSocketClosed`.

### OpenTelemetry Tracing

When enabled (`ALPINE_ENABLE_TRACING=ON`), spans are emitted for HTTP requests, query lifecycle events, and peer communication. Trace context propagates across P2P queries using W3C `traceparent` headers embedded in query packets.

### Structured Logging

`Log::Error/Info/Debug` with correlation IDs (`X-Request-ID`) and optional JSON format. Supports `std::format` syntax: `Log::Info("processed {} queries in {}ms", count, elapsed)`.

---

## Deployment

### Docker Compose

```sh
# 3-node cluster
docker-compose -f docker/docker-compose.yml up

# 5-node cluster (with benchmark nodes)
docker-compose -f docker/docker-compose.yml --profile bench up
```

Exposed ports: `8080` (REST), `8089` (beacon), `8090` (broadcast), `9000` (protocol).

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
- **Health Probes** — Readiness (`/health/ready`) and liveness (`/health/live`) endpoints

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
| `ALPINE_SANITIZER` | — | Comma-separated sanitizers |

### Dependencies (fetched automatically)

| Library | Version | Purpose |
|---------|---------|---------|
| nlohmann/json | 3.11.3 | JSON serialization |
| ASIO | 1.30.2 | Async networking (standalone, no Boost) |
| spdlog | 1.14.1 | Logging backend |
| Catch2 | 3.5.2 | Test framework |
| jwt-cpp | 0.7.0 | JWT handling (when TLS enabled) |
| miniupnpc | 2.2.7 | UPnP client (when UPnP enabled) |
| sqlite3 | 3.45.0 | Persistence (when enabled) |
| opentelemetry-cpp | 1.14.2 | Tracing (when enabled) |
| linenoise | latest | CLI line editing |
| benchmark | 1.8.3 | Benchmarks (when enabled) |

---

## Testing

### Run Tests

```sh
cd build && ctest --output-on-failure --timeout 120
```

### Unit Tests (Catch2)

`test_api_key_auth`, `test_compression`, `test_configuration`, `test_datablock`, `test_error`, `test_http_request`, `test_http_response`, `test_http_router`, `test_json_writer`, `test_packet_auth`, `test_readwritesem`, `test_secure_string`, `test_string_utils`, `test_webhook_dispatcher`

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
      status    ← query status JSON
      results   ← query results JSON
  peers/
    <peer_id>   ← peer info JSON
  stats/
    access      ← access statistics
    popular     ← popular resources
    recent      ← recent queries
```

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

CI pipeline: build + test (3 compiler configs), sanitizers (ASan + UBSan), clang-format, clang-tidy, code coverage, Docker build.

---

## License

Alpine is released under the GNU Lesser General Public License (LGPL). See [LICENCE.txt](LICENCE.txt) for details.
