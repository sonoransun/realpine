# AlpineProtocol and AlpineTransport have a mutual dependency.
# AlpineProtocolHeaders is an INTERFACE target providing just include paths.
# See transport/Alpine/CMakeLists.txt for the counterpart.

add_library(AlpineProtocolHeaders INTERFACE)
target_include_directories(AlpineProtocolHeaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(AlpineProtocol
    AlpineBaseOptionData.cpp
    AlpineBroadcast.cpp
    AlpineExtensionIndex.cpp
    AlpineGroup.cpp
    AlpineGroupMgr.cpp
    AlpineModuleMgr.cpp
    AlpinePacket.cpp
    AlpinePeerLocation.cpp
    AlpinePeerMgr.cpp
    AlpinePeerOptionData.cpp
    AlpinePeerPacket.cpp
    AlpinePeerProfile.cpp
    AlpinePeerProfileIndex.cpp
    AlpineProtocol.cpp
    AlpineProxyOptionData.cpp
    AlpineProxyPacket.cpp
    AlpineQuery.cpp
    AlpineQueryMgr.cpp
    AlpineQueryModuleIndex.cpp
    AlpineQueryOptionData.cpp
    AlpineQueryOptions.cpp
    AlpineQueryPacket.cpp
    AlpineQueryRequest.cpp
    AlpineQueryResults.cpp
    AlpineQueryStatus.cpp
    AlpineRatingEngine.cpp
    AlpineResourceDesc.cpp
    AlpineResourceDescSet.cpp
    AlpineServiceThread.cpp
    AlpineStack.cpp
    AlpineStackConfig.cpp
)
add_library(Alpine::AlpineProtocol ALIAS AlpineProtocol)
target_include_directories(AlpineProtocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineProtocol PUBLIC
    Alpine::Dtcp
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
    Alpine::TransBase
    AlpineTransportHeaders
)
