add_library(TransBase
    AcceptorInterface.cpp
    ConnectorInterface.cpp
    DataBlock.cpp
    DataBuffer.cpp
    MuxInterface.cpp
    StackLinkInterface.cpp
    TransportInterface.cpp
)
add_library(Alpine::TransBase ALIAS TransBase)
target_include_directories(TransBase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TransBase PUBLIC Alpine::AppUtils)
