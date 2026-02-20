# Multi-stage build for Alpine P2P node
# Stage 1: Build AlpineRestBridge from source
# Stage 2: Minimal runtime image

# ---------------------------------------------------------------------------
# Builder stage
# ---------------------------------------------------------------------------
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        g++ \
        cmake \
        make \
        libpthread-stubs0-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -B _build \
        -DCMAKE_BUILD_TYPE=Release \
        -DALPINE_ENABLE_CORBA=OFF \
        -DALPINE_BUILD_TESTS=OFF \
    && cmake --build _build --parallel "$(nproc)"

# Strip debug symbols
RUN strip _build/bin/AlpineRestBridge

# ---------------------------------------------------------------------------
# Runtime stage
# ---------------------------------------------------------------------------
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        iproute2 \
        curl \
        libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

# Copy built binary and shared libraries
COPY --from=builder /src/_build/bin/AlpineRestBridge /usr/local/bin/
COPY --from=builder /src/_build/lib/*.so* /usr/local/lib/

RUN ldconfig

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8080 8089 8090 9000

HEALTHCHECK --interval=5s --timeout=3s --start-period=10s --retries=3 \
    CMD curl -sf http://localhost:${REST_PORT:-8080}/status || exit 1

ENTRYPOINT ["/entrypoint.sh"]
