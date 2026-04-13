# Unit tests using Catch2

# Sources from AlpineRestBridge that we compile directly into test targets
set(REST_SRC_DIR ${CMAKE_SOURCE_DIR}/AlpineRestBridge)

# --- DataBlock tests ---
add_executable(test_datablock test_datablock.cpp)
target_link_libraries(test_datablock PRIVATE
    Catch2::Catch2WithMain
    Alpine::TransBase
)
catch_discover_tests(test_datablock)

# --- HttpRequest tests ---
add_executable(test_http_request
    test_http_request.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
)
target_include_directories(test_http_request PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_http_request PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_http_request)

# --- HttpResponse tests ---
add_executable(test_http_response
    test_http_response.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_http_response PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_http_response PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_http_response)

# --- HttpRouter tests ---
add_executable(test_http_router
    test_http_router.cpp
    ${REST_SRC_DIR}/HttpRouter.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_http_router PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_http_router PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_http_router)

# --- ApiKeyAuth tests ---
add_executable(test_api_key_auth
    test_api_key_auth.cpp
    ${REST_SRC_DIR}/ApiKeyAuth.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_api_key_auth PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_api_key_auth PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_api_key_auth)

# --- ReadWriteSem tests ---
add_executable(test_readwritesem test_readwritesem.cpp)
target_link_libraries(test_readwritesem PRIVATE
    Catch2::Catch2WithMain
    Alpine::ThreadUtils
)
catch_discover_tests(test_readwritesem)

# --- ThreadName tests ---
add_executable(test_thread_name test_thread_name.cpp)
target_link_libraries(test_thread_name PRIVATE
    Catch2::Catch2WithMain
    Alpine::ThreadUtils
)
catch_discover_tests(test_thread_name)

# --- Configuration tests ---
add_executable(test_configuration test_configuration.cpp)
target_link_libraries(test_configuration PRIVATE
    Catch2::Catch2WithMain
    Alpine::ConfigUtils
)
catch_discover_tests(test_configuration)

# --- StringUtils tests ---
add_executable(test_string_utils test_string_utils.cpp)
target_link_libraries(test_string_utils PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_string_utils)

# --- SecureString tests ---
add_executable(test_secure_string test_secure_string.cpp)
target_link_libraries(test_secure_string PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_secure_string)

# --- SecureString zeroization tests ---
add_executable(test_secure_string_zero test_secure_string_zero.cpp)
target_link_libraries(test_secure_string_zero PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_secure_string_zero)

# --- Platform random bytes tests ---
add_executable(test_platform_random test_platform_random.cpp)
target_link_libraries(test_platform_random PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_platform_random)

# --- Error tests ---
add_executable(test_error test_error.cpp)
target_link_libraries(test_error PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_error)

# --- JsonWriter tests ---
add_executable(test_json_writer
    test_json_writer.cpp
    ${REST_SRC_DIR}/JsonWriter.cpp
)
target_include_directories(test_json_writer PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_json_writer PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    nlohmann_json::nlohmann_json
)
catch_discover_tests(test_json_writer)

# --- PacketAuth tests ---
add_executable(test_packet_auth test_packet_auth.cpp)
target_link_libraries(test_packet_auth PRIVATE
    Catch2::Catch2WithMain
    Alpine::AlpineProtocol
    Alpine::ConfigUtils
    Alpine::ThreadUtils
)
if(ALPINE_ENABLE_TLS)
    target_link_libraries(test_packet_auth PRIVATE OpenSSL::Crypto)
    target_compile_definitions(test_packet_auth PRIVATE ALPINE_TLS_ENABLED)
endif()
catch_discover_tests(test_packet_auth)

# --- Compression tests ---
add_executable(test_compression
    test_compression.cpp
    ${REST_SRC_DIR}/Compression.cpp
)
target_include_directories(test_compression PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_compression PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    ZLIB::ZLIB
)
catch_discover_tests(test_compression)

# --- WebhookDispatcher tests ---
add_executable(test_webhook_dispatcher
    test_webhook_dispatcher.cpp
    ${REST_SRC_DIR}/WebhookDispatcher.cpp
)
target_include_directories(test_webhook_dispatcher PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_webhook_dispatcher PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    Alpine::ConfigUtils
    Alpine::ThreadUtils
    Asio::Asio
)
if(ALPINE_ENABLE_TLS)
    target_link_libraries(test_webhook_dispatcher PRIVATE OpenSSL::Crypto)
    target_compile_definitions(test_webhook_dispatcher PRIVATE ALPINE_TLS_ENABLED)
endif()
catch_discover_tests(test_webhook_dispatcher)

# Sources from protocol layer
set(PROTO_SRC_DIR ${CMAKE_SOURCE_DIR}/protocols/Alpine)
set(VFS_SRC_DIR ${CMAKE_SOURCE_DIR}/base/VfsUtils)

# --- CircuitBreaker tests ---
add_executable(test_circuit_breaker test_circuit_breaker.cpp)
target_link_libraries(test_circuit_breaker PRIVATE
    Catch2::Catch2WithMain
    Alpine::AlpineProtocol
    Alpine::ConfigUtils
    Alpine::ThreadUtils
)
catch_discover_tests(test_circuit_breaker)

# --- IpFilter tests ---
add_executable(test_ip_filter test_ip_filter.cpp)
target_link_libraries(test_ip_filter PRIVATE
    Catch2::Catch2WithMain
    Alpine::AlpineProtocol
    Alpine::ConfigUtils
    Alpine::ThreadUtils
)
catch_discover_tests(test_ip_filter)

# --- RbacPolicy tests ---
add_executable(test_rbac_policy
    test_rbac_policy.cpp
    ${REST_SRC_DIR}/RbacPolicy.cpp
)
target_include_directories(test_rbac_policy PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rbac_policy PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    nlohmann_json::nlohmann_json
)
catch_discover_tests(test_rbac_policy)

# --- RateLimiter tests ---
add_executable(test_rate_limiter
    test_rate_limiter.cpp
    ${REST_SRC_DIR}/RateLimiter.cpp
)
target_include_directories(test_rate_limiter PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_rate_limiter PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    Alpine::ThreadUtils
    Asio::Asio
)
catch_discover_tests(test_rate_limiter)

# --- QueryCache tests ---
add_executable(test_query_cache
    test_query_cache.cpp
    ${PROTO_SRC_DIR}/QueryCache.cpp
)
target_include_directories(test_query_cache PRIVATE ${PROTO_SRC_DIR})
target_link_libraries(test_query_cache PRIVATE
    Catch2::Catch2WithMain
    Alpine::ThreadUtils
    Alpine::AppUtils
)
catch_discover_tests(test_query_cache)

# --- AuditLog tests ---
add_executable(test_audit_log test_audit_log.cpp)
target_link_libraries(test_audit_log PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
    nlohmann_json::nlohmann_json
)
catch_discover_tests(test_audit_log)

# --- AccessTracker tests ---
add_executable(test_access_tracker
    test_access_tracker.cpp
    ${VFS_SRC_DIR}/AccessTracker.cpp
)
target_include_directories(test_access_tracker PRIVATE ${VFS_SRC_DIR})
target_link_libraries(test_access_tracker PRIVATE
    Catch2::Catch2WithMain
    Alpine::ThreadUtils
    Alpine::AppUtils
    nlohmann_json::nlohmann_json
)
catch_discover_tests(test_access_tracker)

# --- AlpinePacket tests ---
add_executable(test_alpine_packet test_alpine_packet.cpp)
target_link_libraries(test_alpine_packet PRIVATE
    Catch2::Catch2WithMain
    Alpine::AlpineProtocol
    Alpine::ConfigUtils
    Alpine::ThreadUtils
)
catch_discover_tests(test_alpine_packet)

# --- PollSet tests ---
add_executable(test_poll_set test_poll_set.cpp)
target_link_libraries(test_poll_set PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_poll_set)

# --- UdpConnection tests ---
add_executable(test_udp_connection test_udp_connection.cpp)
target_link_libraries(test_udp_connection PRIVATE
    Catch2::Catch2WithMain
    Alpine::NetUtils
    Alpine::AppUtils
)
catch_discover_tests(test_udp_connection)

# --- UdpConnection SO_REUSEPORT tests ---
add_executable(test_udp_reuseport test_udp_reuseport.cpp)
target_link_libraries(test_udp_reuseport PRIVATE
    Catch2::Catch2WithMain
    Alpine::NetUtils
    Alpine::AppUtils
)
catch_discover_tests(test_udp_reuseport)

# --- AlpineRatingEngine tests ---
add_executable(test_rating_engine test_rating_engine.cpp)
target_link_libraries(test_rating_engine PRIVATE
    Catch2::Catch2WithMain
    Alpine::AlpineProtocol
    Alpine::ConfigUtils
    Alpine::ThreadUtils
)
catch_discover_tests(test_rating_engine)

# --- UnicastWifiConnection tests ---
add_executable(test_unicast_wifi_connection test_unicast_wifi_connection.cpp)
target_link_libraries(test_unicast_wifi_connection PRIVATE
    Catch2::Catch2WithMain
    Alpine::NetUtils
    Alpine::AppUtils
)
catch_discover_tests(test_unicast_wifi_connection)

# --- SDR radio frame encoding/decoding tests ---
add_executable(test_sdr_radio_frame test_sdr_radio_frame.cpp)
target_link_libraries(test_sdr_radio_frame PRIVATE
    Catch2::Catch2WithMain
    Alpine::NetUtils
    Alpine::AppUtils
)
catch_discover_tests(test_sdr_radio_frame)

# --- GfskDemodulator tests (requires RTL-SDR build option) ---
if(ALPINE_ENABLE_RTLSDR)
    add_executable(test_gfsk_demodulator test_gfsk_demodulator.cpp)
    target_link_libraries(test_gfsk_demodulator PRIVATE
        Catch2::Catch2WithMain
        Alpine::NetUtils
        Alpine::AppUtils
    )
    catch_discover_tests(test_gfsk_demodulator)
endif()
