///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <AppState.h>
#include <AsyncRpcClient.h>
#include <GuiPanels.h>
#include <GuiHelpers.h>

#include <chrono>
#include <cstdio>


static void
glfwErrorCallback (int error, const char * description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int
main (int, char **)
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
        return 1;

    // GL context hints
#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char * glslVersion = "#version 150";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    const char * glslVersion = "#version 130";
#endif

    GLFWwindow * window = glfwCreateWindow(1280, 800,
                              "Alpine Server Management", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync

    // Dear ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Slight style tweaks
    ImGuiStyle & style = ImGui::GetStyle();
    style.FrameRounding = 3.0f;
    style.GrabRounding  = 3.0f;
    style.WindowRounding = 4.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    // Application state
    AppState state;
    AsyncRpcClient client;

    addLog(state, LogLevel::Info, "Alpine GUI started");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Full-window ImGui panel
        int winW, winH;
        glfwGetFramebufferSize(window, &winW, &winH);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(winW),
                                        static_cast<float>(winH)));
        ImGui::Begin("##Main", nullptr,
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Connection bar at top
        drawConnectionBar(state, client);

        // Main content area (tabs) — leave room for log panel
        float logHeight = 160.0f;
        float availH = ImGui::GetContentRegionAvail().y;

        ImGui::BeginChild("TabArea", ImVec2(0, availH - logHeight), false);
        drawMainTabs(state, client);
        ImGui::EndChild();

        // Log panel at bottom
        drawLogPanel(state);

        ImGui::End();

        // Check pending async call
        if (state.callInProgress && state.pendingCall.has_value())
        {
            auto & fut = state.pendingCall.value();
            if (fut.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
            {
                bool ok = fut.get();
                state.pendingCall.reset();
                state.callInProgress = false;

                // Route result to the correct state field
                const auto & target = state.callTarget;
                const auto & result = state.pendingResult;

                if (ok)
                {
                    addLog(state, LogLevel::Info, "Call succeeded");

                    if (target == "status")           state.statusResult = result;
                    else if (target == "peers")       state.peersResult = result;
                    else if (target == "peerInfo")    state.peerInfoResult = result;
                    else if (target == "query")       state.queryResult = result;
                    else if (target == "queryStatus") state.queryStatusResult = result;
                    else if (target == "groups")      state.groupsResult = result;
                    else if (target == "groupInfo")   state.groupInfoResult = result;
                    else if (target == "modules")     state.modulesResult = result;
                    else if (target == "moduleInfo")  state.moduleInfoResult = result;
                    else if (target == "excludedHosts")   state.excludedHostsResult = result;
                    else if (target == "excludedSubnets") state.excludedSubnetsResult = result;
                    else if (target == "raw")         state.rawResult = result;
                }
                else
                {
                    string msg = "Call failed";
                    if (!result.empty())
                        msg += ": " + result;
                    addLog(state, LogLevel::Error, msg);

                    if (target == "raw")
                        state.rawResult = result.empty() ? "Error: call failed" : result;
                }
            }
        }

        // Render
        ImGui::Render();
        glViewport(0, 0, winW, winH);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
