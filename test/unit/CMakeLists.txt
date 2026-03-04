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
