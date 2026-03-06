# Multi-stage build for Alpine P2P node
# Stage 1: Build AlpineRestBridge from source
# Stage 2: Minimal runtime image
#
# Supports multi-arch builds via docker buildx:
#   docker buildx build --platform linux/amd64,linux/arm64 -f docker/Dockerfile .

# ---------------------------------------------------------------------------
# Builder stage
# ---------------------------------------------------------------------------
FROM --platform=$BUILDPLATFORM ubuntu:24.04 AS builder

ARG TARGETPLATFORM
ARG BUILDPLATFORM
ARG TARGETARCH

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        g++ \
        cmake \
        make \
        libpthread-stubs0-dev \
    && rm -rf /var/lib/apt/lists/*

# Install cross-compilation toolchain when cross-building
RUN if [ "$BUILDPLATFORM" != "$TARGETPLATFORM" ]; then \
        apt-get update && apt-get install -y --no-install-recommends \
            crossbuild-essential-arm64 \
        && rm -rf /var/lib/apt/lists/*; \
    fi

WORKDIR /src
COPY . .

# Set cross-compilation cmake flags if needed
RUN if [ "$TARGETARCH" = "arm64" ] && [ "$BUILDPLATFORM" != "$TARGETPLATFORM" ]; then \
        cmake -B _build \
            -DCMAKE_BUILD_TYPE=Release \
            -DALPINE_ENABLE_CORBA=OFF \
            -DALPINE_BUILD_TESTS=OFF \
            -DCMAKE_SYSTEM_NAME=Linux \
            -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
            -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
            -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++; \
    else \
        cmake -B _build \
            -DCMAKE_BUILD_TYPE=Release \
            -DALPINE_ENABLE_CORBA=OFF \
            -DALPINE_BUILD_TESTS=OFF; \
    fi \
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
