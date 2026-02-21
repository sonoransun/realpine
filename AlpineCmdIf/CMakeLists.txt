add_executable(AlpineCmdIf
    AlpineConfig.cpp
    AlpineCmdIf.cpp
    JsonRpcClient.cpp
    ../AlpineRestBridge/JsonWriter.cpp
    ../AlpineRestBridge/JsonReader.cpp
)
target_include_directories(AlpineCmdIf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge
)
target_link_libraries(AlpineCmdIf PRIVATE
    Alpine::ApplCore
    Alpine::AppUtils
    Alpine::NetUtils
    Alpine::ConfigUtils
)
