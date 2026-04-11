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

# AlpineGui pulls in vendored imgui sources that were not written with
# our strict warning set in mind.  We cannot recompile third-party code
# with our -Werror knob without patching it, so disable specific -Werror=
# promotions on this target only.  Alpine's own GUI sources still build
# with the full warning set — these -Wno-error= directives only downgrade
# the listed warnings back to warnings for this one target.
target_compile_options(AlpineGui PRIVATE
    -Wno-error=double-promotion
    -Wno-error=cast-qual
    -Wno-error=format-nonliteral
    -Wno-error=old-style-cast
    -Wno-error=unused-parameter
    -Wno-error=unused-but-set-variable
    -Wno-error=sign-compare
    -Wno-error=implicit-fallthrough)

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
