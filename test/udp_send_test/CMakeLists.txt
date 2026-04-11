add_executable(udpSendTest
    udpSendTest.cpp
)
target_include_directories(udpSendTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(udpSendTest PRIVATE Alpine::AppUtils Alpine::NetUtils Alpine::ThreadUtils)

# udpSendTest is fire-and-forget: it opens a UDP socket and blasts N packets
# to the destination, then exits.  No receiver is required (UDP datagrams to
# a closed port are silently dropped at the IP layer on loopback).  Args:
#   <logFile> <debugLevel 1-4> <ipAddr> <port> <sendCount> <packetSize>
add_test(NAME UdpSend::loopback_smoke
         COMMAND udpSendTest /tmp/alpine-udpsend.log 2 127.0.0.1 38123 16 64
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_tests_properties(UdpSend::loopback_smoke PROPERTIES TIMEOUT 30)
