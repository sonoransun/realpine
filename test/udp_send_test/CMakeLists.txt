add_executable(udpSendTest
    udpSendTest.cpp
)
target_include_directories(udpSendTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(udpSendTest PRIVATE Alpine::AppUtils Alpine::NetUtils Alpine::ThreadUtils)
