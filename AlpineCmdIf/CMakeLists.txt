if(NOT ALPINE_ENABLE_CORBA)
    message(STATUS "Skipping AlpineCmdIf (requires CORBA)")
    return()
endif()

add_executable(AlpineCmdIf
    AlpineConfig.cpp
    AlpineCmdIf.cpp
)
target_include_directories(AlpineCmdIf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineCmdIf PRIVATE
    Alpine::ApplCore
    Alpine::CorbaServant
    Alpine::CorbaUtils
    Alpine::AlpineCorbaClient
    Alpine::AlpineProtocol
    Alpine::AlpineTransport
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)
