add_executable(dtcpStackTestServer
    dtcpStackTestServer.cpp
    DtcpStackTest.cpp
    ServerTestConfig.cpp
)
target_include_directories(dtcpStackTestServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dtcpStackTestServer PRIVATE
    Alpine::ApplCore
    Alpine::Stack
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)

add_executable(dtcpStackTestClient
    dtcpStackTestClient.cpp
    DtcpStackTest.cpp
    ClientTestConfig.cpp
)
target_include_directories(dtcpStackTestClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(dtcpStackTestClient PRIVATE
    Alpine::ApplCore
    Alpine::Stack
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)

# Pair smoke test: dtcpStackTestServer ends with `while (true) sleep(3600)`
# and dtcpStackTestClient does the same after issuing connection requests,
# so the wrapper script verifies both stay alive after a brief startup
# window then tears them down.
add_test(NAME DtcpStack::pair
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_pair.sh)
# Legacy pair test — see test/tcp_conn_test/CMakeLists.txt for the same
# rationale.  Enabled only when ALPINE_ENABLE_LEGACY_PAIR_TESTS=ON.
set_tests_properties(DtcpStack::pair PROPERTIES
    TIMEOUT 30
    DISABLED $<NOT:$<BOOL:${ALPINE_ENABLE_LEGACY_PAIR_TESTS}>>
    ENVIRONMENT "ALPINE_BIN_DIR=${CMAKE_BINARY_DIR}/bin")
