add_executable(tcpServer
    TcpClientThread.cpp
    tcpServer.cpp
    TcpServerConfig.cpp
)
target_include_directories(tcpServer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tcpServer PRIVATE
    Alpine::ApplCore
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)

add_executable(tcpClient
    TcpClientThread.cpp
    tcpClient.cpp
    TcpClientConfig.cpp
    TcpClientConnector.cpp
)
target_include_directories(tcpClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tcpClient PRIVATE
    Alpine::ApplCore
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)

# Pair smoke test: tcpServer accepts forever and tcpClient loops on
# processEvents forever, so the wrapper script verifies both processes
# stay alive after a brief startup window then tears them down.
#
# The legacy binaries require a `server.cfg` / `client.cfg` file in CWD plus
# additional ConfigUtils plumbing that ApplCore::initialize does not set up in
# isolation.  The pair test is disabled by default until the legacy harnesses
# are modernized; flip ALPINE_ENABLE_LEGACY_PAIR_TESTS to run it manually.
add_test(NAME TcpConn::pair
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/run_pair.sh)
set_tests_properties(TcpConn::pair PROPERTIES
    TIMEOUT 30
    DISABLED $<NOT:$<BOOL:${ALPINE_ENABLE_LEGACY_PAIR_TESTS}>>
    ENVIRONMENT "ALPINE_BIN_DIR=${CMAKE_BINARY_DIR}/bin")
