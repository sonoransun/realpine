# GLFW options — build only the library, nothing else
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

find_package(OpenGL REQUIRED)

add_executable(AlpineGui
    AlpineGui.cpp
    AsyncRpcClient.cpp
    GuiPanels.cpp
    GuiHelpers.cpp
    # ImGui core
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_demo.cpp
    # ImGui backends
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
    # JSON from AlpineRestBridge
    ../AlpineRestBridge/JsonWriter.cpp
    ../AlpineRestBridge/JsonReader.cpp
)

# Silence macOS OpenGL deprecation warnings
target_compile_definitions(AlpineGui PRIVATE GL_SILENCE_DEPRECATION)

target_include_directories(AlpineGui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends
    ${CMAKE_SOURCE_DIR}/AlpineRestBridge
)

target_link_libraries(AlpineGui PRIVATE
    Alpine::Flags
    Alpine::NetUtils
    nlohmann_json::nlohmann_json
    glfw
    OpenGL::GL
)

# macOS frameworks required by GLFW
if(APPLE)
    target_link_libraries(AlpineGui PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
