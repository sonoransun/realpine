add_library(AlpineCorbaClient
    AlpineCorbaClient.cpp
    AlpineGroupMgmt.cpp
    AlpineModuleMgmt.cpp
    AlpinePeerMgmt.cpp
    AlpineQuery.cpp
    AlpineStackMgmt.cpp
    AlpineStackStatus.cpp
    DtcpPeerMgmt.cpp
    DtcpStackMgmt.cpp
    DtcpStackStatus.cpp
)
add_library(Alpine::AlpineCorbaClient ALIAS AlpineCorbaClient)
target_include_directories(AlpineCorbaClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineCorbaClient PUBLIC
    Alpine::AlpineIdl
    Alpine::CorbaUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
)
