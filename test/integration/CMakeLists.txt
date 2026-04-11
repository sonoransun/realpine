# Integration tests using Catch2

set(REST_SRC_DIR ${CMAKE_SOURCE_DIR}/AlpineRestBridge)

# --- Query lifecycle integration test ---
add_executable(test_rest_query_lifecycle
    test_rest_query_lifecycle.cpp
    ${REST_SRC_DIR}/HttpRouter.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_rest_query_lifecycle PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rest_query_lifecycle PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_rest_query_lifecycle)

# --- Peer endpoint integration test ---
add_executable(test_rest_peer_endpoints
    test_rest_peer_endpoints.cpp
    ${REST_SRC_DIR}/HttpRouter.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_rest_peer_endpoints PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rest_peer_endpoints PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_rest_peer_endpoints)

# --- Auth flow integration test ---
add_executable(test_rest_auth_flow
    test_rest_auth_flow.cpp
    ${REST_SRC_DIR}/HttpRouter.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/ApiKeyAuth.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_rest_auth_flow PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rest_auth_flow PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_rest_auth_flow)

# --- Rate limiting integration test ---
add_executable(test_rest_rate_limiting
    test_rest_rate_limiting.cpp
    ${REST_SRC_DIR}/RateLimiter.cpp
)
target_include_directories(test_rest_rate_limiting PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rest_rate_limiting PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    Asio::Asio
)
catch_discover_tests(test_rest_rate_limiting)

# --- Real HttpServer integration test (binds to a loopback ephemeral port) ---
add_executable(test_rest_real_server
    test_rest_real_server.cpp
    ${REST_SRC_DIR}/HttpServer.cpp
    ${REST_SRC_DIR}/HttpRouter.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/Compression.cpp
    ${REST_SRC_DIR}/RateLimiter.cpp
    ${REST_SRC_DIR}/RestBridgeConfig.cpp
    ${REST_SRC_DIR}/MetricsHandler.cpp
    ${REST_SRC_DIR}/WebSocketSession.cpp
)
target_include_directories(test_rest_real_server PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rest_real_server PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    Alpine::ConfigUtils
    Asio::Asio
    ZLIB::ZLIB
)
catch_discover_tests(test_rest_real_server)
