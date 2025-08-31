#include "renderer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "shader.h"
#include "structs.h"

Renderer::Renderer() {
    assert(glfwInit() && "GLFW3 did not initialize");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    this->window =
        glfwCreateWindow(800, 600, "PSX by *^hrs", NULL, NULL);
    assert(window && "Window did not create");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // GLAD initialization
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) &&
           "Failed to initialize GLAD\n");
    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    this->program = new Shader("vertex.glsl", "fragment.glsl");
    this->color_buf = new Buffer<Color>();
    this->pos_buf = new Buffer<Position>();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    this->pos_buf->bind();
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_SHORT, 0, nullptr);
    this->color_buf->bind();
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, nullptr);
    glBindVertexArray(0);
    this->nvertices = 0;
    this->run();
}

void Renderer::push_triangle(Position positions[3],
                             Color colors[3]) {
    if ((this->nvertices + 3) > this->VERTEX_BUFFER_LEN) {
        printf("Vertex attribute buffers full, forcing draw\n");
        this->draw();
    }

    for (int i = 0; i < 3; i++) {
        this->pos_buf->set(this->nvertices, positions[i]);
        this->color_buf->set(this->nvertices, colors[i]);
        this->nvertices += 1;
    }
}

Renderer::~Renderer() {
    delete this->program;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::draw() {
    this->color_buf->flush();
    this->pos_buf->flush();

    this->program->use();
    glBindVertexArray(this->vao);
    glDrawArrays(GL_TRIANGLES, 0,
                 static_cast<GLsizei>(nvertices));

    glFlush();
    nvertices = 0;
}
void Renderer::run() {
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->draw();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Menu");
        ImGui::End();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
