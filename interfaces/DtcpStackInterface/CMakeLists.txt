add_library(DtcpStackInterface
    DtcpStackInterface.cpp
)
add_library(Alpine::DtcpStackInterface ALIAS DtcpStackInterface)
target_include_directories(DtcpStackInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(DtcpStackInterface PUBLIC
    Alpine::Dtcp
    Alpine::AppUtils
    Alpine::NetUtils
)
