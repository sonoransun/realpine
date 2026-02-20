add_library(AlpineCorbaServer
    AlpineCorbaServer.cpp
    AlpineCorba_impl.cpp
    AlpineGroupMgmt_impl.cpp
    AlpineModuleMgmt_impl.cpp
    AlpinePeerMgmt_impl.cpp
    AlpineQuery_impl.cpp
    AlpineStackMgmt_impl.cpp
    AlpineStackStatus_impl.cpp
    DtcpPeerMgmt_impl.cpp
    DtcpStackMgmt_impl.cpp
    DtcpStackStatus_impl.cpp
)
add_library(Alpine::AlpineCorbaServer ALIAS AlpineCorbaServer)
target_include_directories(AlpineCorbaServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineCorbaServer PUBLIC
    Alpine::AlpineIdl
    Alpine::CorbaUtils
    Alpine::DtcpStackInterface
    Alpine::AppUtils
    Alpine::ThreadUtils
)
