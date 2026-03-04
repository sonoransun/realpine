if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "Fuzzing requires Clang compiler, skipping fuzz targets")
    return()
endif()

add_executable(fuzz_http_parse
    fuzz_http_parse.cpp
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge/HttpRequest.cpp
)
target_include_directories(fuzz_http_parse PRIVATE
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge
)
target_link_libraries(fuzz_http_parse PRIVATE
    Alpine::Flags
    Alpine::AppUtils
)
target_compile_options(fuzz_http_parse PRIVATE -fsanitize=fuzzer,address -fno-omit-frame-pointer)
target_link_options(fuzz_http_parse PRIVATE -fsanitize=fuzzer,address)

add_executable(fuzz_http_router
    fuzz_http_router.cpp
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge/HttpRequest.cpp
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge/HttpResponse.cpp
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge/HttpRouter.cpp
)
target_include_directories(fuzz_http_router PRIVATE
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge
)
target_link_libraries(fuzz_http_router PRIVATE
    Alpine::Flags
    Alpine::AppUtils
)
target_compile_options(fuzz_http_router PRIVATE -fsanitize=fuzzer,address -fno-omit-frame-pointer)
target_link_options(fuzz_http_router PRIVATE -fsanitize=fuzzer,address)
