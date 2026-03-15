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
)
target_include_directories(test_http_response PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_http_response PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_http_response)

# --- HttpRouter tests ---
add_executable(test_http_router
    test_http_router.cpp
    ${REST_SRC_DIR}/HttpRouter.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
)
target_include_directories(test_http_router PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_http_router PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_http_router)

# --- ApiKeyAuth tests ---
add_executable(test_api_key_auth
    test_api_key_auth.cpp
    ${REST_SRC_DIR}/ApiKeyAuth.cpp
    ${REST_SRC_DIR}/HttpRequest.cpp
    ${REST_SRC_DIR}/HttpResponse.cpp
)
target_include_directories(test_api_key_auth PRIVATE ${REST_SRC_DIR})
target_link_libraries(test_api_key_auth PRIVATE
    Catch2::Catch2WithMain
    Alpine::AppUtils
)
catch_discover_tests(test_api_key_auth)

# --- ReadWriteSem tests ---
add_executable(test_readwritesem test_readwritesem.cpp)
target_link_libraries(test_readwritesem PRIVATE
    Catch2::Catch2WithMain
    Alpine::ThreadUtils
)
catch_discover_tests(test_readwritesem)

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
