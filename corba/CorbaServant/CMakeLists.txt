add_library(CorbaServant
    CorbaServant.cpp
    AlpineStackMgmtIntf.cpp
    AlpineGroupMgmtIntf.cpp
    AlpineModuleMgmtIntf.cpp
    AlpinePeerMgmtIntf.cpp
    AlpineQueryIntf.cpp
    AlpineStackStatusIntf.cpp
    DtcpStackMgmtIntf.cpp
    DtcpPeerMgmtIntf.cpp
    DtcpStackStatusIntf.cpp
)
add_library(Alpine::CorbaServant ALIAS CorbaServant)
target_include_directories(CorbaServant PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CorbaServant PUBLIC
    Alpine::AlpineCorbaClient
    Alpine::CorbaUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
)
