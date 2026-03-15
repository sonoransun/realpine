add_executable(AlpineRestBridge
    JsonWriter.cpp
    JsonReader.cpp
    XmlWriter.cpp
    HttpRequest.cpp
    HttpResponse.cpp
    HttpRouter.cpp
    HttpServer.cpp
    Compression.cpp
    RateLimiter.cpp
    ApiKeyAuth.cpp
    RestBridgeConfig.cpp
    DiscoveryBeacon.cpp
    BroadcastQueryHandler.cpp
    TorTunnel.cpp
    ContentStore.cpp
    DlnaHandler.cpp
    MediaStreamer.cpp
    DlnaServer.cpp
    SsdpService.cpp
    MdnsService.cpp
    QueryHandler.cpp
    PeerHandler.cpp
    StatusHandler.cpp
    AuthHandler.cpp
    MetricsHandler.cpp
    ApiDocsHandler.cpp
    AdminHandler.cpp
    WebSocketSession.cpp
    ClusterCoordinator.cpp
    WebhookDispatcher.cpp
    AlpineRestBridge.cpp
)
target_include_directories(AlpineRestBridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AlpineRestBridge PRIVATE
    Alpine::ApplCore
    Alpine::AlpineStackInterface
    Alpine::DtcpStackInterface
    Alpine::AlpineProtocol
    Alpine::AlpineTransport
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ThreadUtils
    Alpine::ConfigUtils
    nlohmann_json::nlohmann_json
    Asio::Asio
    ZLIB::ZLIB
)

if(ALPINE_ENABLE_TLS)
    target_link_libraries(AlpineRestBridge PRIVATE jwt-cpp::jwt-cpp OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(AlpineRestBridge PRIVATE ALPINE_TLS_ENABLED)
endif()
if(ALPINE_ENABLE_FUSE)
    target_link_libraries(AlpineRestBridge PRIVATE Alpine::VfsUtils)
    target_compile_definitions(AlpineRestBridge PRIVATE ALPINE_FUSE_ENABLED)
endif()
