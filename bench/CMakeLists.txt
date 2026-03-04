add_executable(bench_datablock bench_datablock.cpp)
target_link_libraries(bench_datablock PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    Alpine::TransBase
    Alpine::AppUtils
    Alpine::Flags
)

add_executable(bench_http_parse bench_http_parse.cpp)
target_link_libraries(bench_http_parse PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
    Alpine::Flags
)
target_include_directories(bench_http_parse PRIVATE
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge
)
target_sources(bench_http_parse PRIVATE
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge/HttpRequest.cpp
)
