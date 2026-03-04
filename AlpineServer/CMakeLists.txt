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
    ../AlpineRestBridge/ApiKeyAuth.cpp
    ../AlpineRestBridge/RateLimiter.cpp
    ../AlpineRestBridge/RestBridgeConfig.cpp
    ../AlpineRestBridge/WebSocketSession.cpp
    ../AlpineRestBridge/ClusterCoordinator.cpp
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
    nlohmann_json::nlohmann_json
    Asio::Asio
)
if(ALPINE_ENABLE_CORBA)
    target_link_libraries(AlpineServer PRIVATE
        Alpine::CorbaAdmin
        Alpine::CorbaUtils
        Alpine::AlpineCorbaServer
    )
endif()
if(ALPINE_ENABLE_FUSE)
    target_link_libraries(AlpineServer PRIVATE Alpine::VfsUtils)
    target_compile_definitions(AlpineServer PRIVATE ALPINE_FUSE_ENABLED)
endif()
