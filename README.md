# Alpine

A decentralized peer-to-peer resource discovery and distributed querying platform built in modern C++23.

Alpine enables autonomous nodes to discover each other, share resource metadata, and retrieve content across a network without relying on any central directory or coordinator. Peers locate one another through broadcast-based discovery, exchange queries in parallel, and aggregate results from across the network — all driven by adaptive quality metrics that learn which peers are most reliable over time.

## Core Concepts

### Decentralized Resource Discovery

Alpine's discovery model operates without a central index. Every node is both a producer and a consumer of resources:

- **Broadcast discovery** — Nodes announce their presence and discover peers through UDP multicast and broadcast mechanisms. No registration server is required.
- **Distributed querying** — A query originator broadcasts a `queryDiscover` packet to a group of peers. Peers that hold matching resources respond with a `queryOffer` indicating their hit count. The originator then sends `queryRequest` messages to the most promising peers and collects `queryReply` packets containing full resource descriptions.
- **Peer quality tracking** — Each node maintains per-peer quality scores based on response rates and reliability. Future queries are routed preferentially toward peers that have historically provided fast, accurate results.
- **Peer groups** — Peers can be organized into logical groups with independent quality profiles, enabling targeted queries to subsets of the network.

### Protocol Stack

Alpine uses a layered packet architecture:

```
Application Query
    └─ AlpinePacket (discovery, offer, request, reply, peer list, proxy)
          └─ DtcpPacket (connection setup, reliable/unreliable data, ACKs)
                └─ UDP broadcast  or  TCP stream
```

**Dtcp (Direct TCP Protocol)** provides the transport layer with connection multiplexing, reliable delivery with acknowledgments, and connection suspend/resume. **Alpine Protocol** sits above Dtcp and implements the query/response lifecycle, peer list exchange, and proxy routing.

### Service Discovery Integration

Beyond its own broadcast protocol, Alpine integrates with standard service discovery mechanisms:

- **mDNS / DNS-SD** — Announces Alpine services on the local network via multicast DNS (port 5353), enabling discovery by any Bonjour/Avahi-aware client.
- **SSDP / UPnP** — Participates in UPnP device discovery on the standard multicast group (239.255.255.250:1900), responding to M-SEARCH requests and sending periodic NOTIFY announcements.
- **WiFi discovery** — Leverages WiFi-layer beacon mechanisms for peer detection in wireless environments.

## Application Capabilities

### Multiple Access Interfaces

Alpine exposes its functionality through several interfaces suited to different integration scenarios:

| Interface | Description |
|-----------|-------------|
| **C++ API** (`AlpineStackInterface`) | Direct programmatic access to queries, groups, peers, modules, and results |
| **REST / HTTP** (`AlpineRestBridge`) | JSON-based HTTP API with endpoints for `/query`, `/peer`, and `/status` |
| **Command-line** (`AlpineCmdIf`) | Interactive CLI for query testing, peer management, and configuration |
| **CORBA** (optional) | Remote ORB-based administration for distributed management |

### REST API

The REST bridge runs a multi-threaded HTTP server with structured routing:

- `POST /query/start` — Initiate a new distributed query
- `GET /query/{id}` — Check query status
- `GET /query/{id}/results` — Retrieve aggregated results
- `DELETE /query/{id}` — Cancel an active query
- `GET /peer` — List all discovered peers
- `GET /peer/{id}` — Get details for a specific peer
- `GET /status` — Server and system status

### Media and Content Services

- **DLNA server** — Streams discovered media content to DLNA-compatible renderers
- **Media streaming** — Direct content streaming from peers
- **Content store** — Local content storage and retrieval

### Module System

Alpine supports a plugin architecture through `AlpineModuleInterface`:

- **Query modules** — Custom query handlers and result processors
- **Extension modules** — Additional functionality loaded at runtime
- **Transport modules** — Client/server transport plugins loaded via `DynamicLoader`

## Architecture

```
realpine/
├── base/                   Core libraries
│   ├── AppUtils/             String, logging, hashing, callbacks
│   ├── SysUtils/             File, process, dynamic loading
│   ├── ThreadUtils/          Threads, mutexes, read-write locks
│   ├── NetUtils/             TCP, UDP, multicast, WiFi discovery
│   └── ConfigUtils/          Configuration management
├── protocols/
│   └── Alpine/             Alpine P2P protocol implementation
├── transport/
│   ├── TransBase/            Transport interfaces
│   ├── Dtcp/                 Direct TCP protocol
│   └── Alpine/               Alpine transport (Dtcp + Alpine protocol)
├── applcore/               Application core framework
├── interfaces/
│   ├── AlpineStackInterface/ Primary C++ API
│   └── DtcpStackInterface/   Transport-level API
├── AlpineServer/           Standalone server daemon
├── AlpineCmdIf/            Command-line client
├── AlpineRestBridge/       REST API bridge with DLNA, mDNS, SSDP
├── docker/                 Docker deployment configurations
└── test/                   Test programs
```

## Building

Requires CMake 3.25+ and a C++23-capable compiler.

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Optional flags:

| Flag | Description |
|------|-------------|
| `ALPINE_ENABLE_CORBA=ON` | Enable CORBA remote management (requires ACE/TAO) |
| `CMAKE_BUILD_TYPE=Profile` | Build with profiling support |

Sanitizer builds (address, undefined, thread, memory) are supported through standard CMake sanitizer configuration.

## Deployment

### Standalone Server

```sh
./AlpineServer
```

The server daemon handles signal-based lifecycle management (graceful shutdown on SIGTERM) and supports configuration hot-reload.

### Docker

```sh
docker-compose up
```

Docker configurations are provided for both Linux and Windows environments.

## Security

- Duplicate packet detection with configurable thresholds
- Configurable packet size limits for queries, resource descriptions, and peer lists
- Peer banning for misbehaving nodes
- Bad packet tracking with per-peer counters
- Reliable transfer failure monitoring

## License

MIT

## Copyright

Copyright (c) 2026 sonoransun
