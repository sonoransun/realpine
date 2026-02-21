add_executable(AlpineServer
    AlpineConfig.cpp
    AlpineServer.cpp
    ServerSigMethods.cpp
    JsonRpcHandler.cpp
    ../AlpineRestBridge/HttpServer.cpp
    ../AlpineRestBridge/HttpRouter.cpp
    ../AlpineRestBridge/HttpRequest.cpp
    ../AlpineRestBridge/HttpResponse.cpp
    ../AlpineRestBridge/JsonWriter.cpp
    ../AlpineRestBridge/JsonReader.cpp
)
target_include_directories(AlpineServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge
)
target_link_libraries(AlpineServer PRIVATE
    Alpine::ApplCore
    Alpine::AlpineStackInterface
    Alpine::DtcpStackInterface
    Alpine::AlpineProtocol
    Alpine::AlpineTransport
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)
if(ALPINE_ENABLE_CORBA)
    target_link_libraries(AlpineServer PRIVATE
        Alpine::CorbaAdmin
        Alpine::CorbaUtils
        Alpine::AlpineCorbaServer
    )
endif()
