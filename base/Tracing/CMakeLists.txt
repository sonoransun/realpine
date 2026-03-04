add_library(Tracing
    Tracing.cpp
)
add_library(Alpine::Tracing ALIAS Tracing)
target_include_directories(Tracing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Tracing PUBLIC Alpine::AppUtils)

if(ALPINE_ENABLE_TRACING)
    target_compile_definitions(Tracing PUBLIC ALPINE_TRACING_ENABLED)
    target_link_libraries(Tracing PUBLIC
        opentelemetry_trace
        opentelemetry_exporter_otlp_http
        opentelemetry_resources
    )
endif()
