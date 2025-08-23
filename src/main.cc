#include "bios.h"
#include "cpu.h"
#include "gpu.h"
#include "interconnect.h"
#include "ram.h"
#include "dma.h"
#include "commandbuffer.h"
#include "glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int main(void) {
    Bios bios("SCPH1001.BIN");
    RAM ram;
    Dma dma;
    CommmandBuffer cb;
    GPU gpu(&cb);
    Interconnect inter(&bios, &ram, &dma, &gpu);
    CPU *cpu = new CPU(&inter);

    // GLFW
    assert(glfwInit() && "GLFW3 did not initialize");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    GLFWwindow *window =
        glfwCreateWindow(800, 600, "PSX by *^hrs", NULL, NULL);

    assert(window && "Window did not create");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // GLAD
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) &&
           "Failed to initialize GLAD\n");
    glViewport(0, 0, 800, 600);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(119.0f / 255.0f, 168.0f / 255.0f, 1.0f,
                     1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cpu->run();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Menu");
        ImGui::End();
        ImGui::Render();
        auto ImGuiImplOpenGL3RenderDrawData =
            ImGui_ImplOpenGL3_RenderDrawData;
        ImGuiImplOpenGL3RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    delete cpu;
    return EXIT_SUCCESS;
}
