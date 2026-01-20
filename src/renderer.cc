#include "renderer.h"
#include "GLFW/glfw3.h"
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
        glfwCreateWindow(800, 600, "main", NULL, NULL);
    assert(window && "Window did not create");

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    // GLAD initialization
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) &&
           "Failed to initialize GLAD\n");
    this->vendor = (const char *)glGetString(GL_VENDOR);
    this->renderer = (const char *)glGetString(GL_RENDERER);

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    // Viewport
    int fb_w, fb_h;
    glfwGetFramebufferSize(window, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);

    this->program = new Shader("vertex.glsl", "fragment.glsl");
    this->color_buf = new Buffer<Color>();
    this->pos_buf = new Buffer<Position>();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    this->pos_buf->bind();
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 2, GL_SHORT, 0, (void *)0);
    this->color_buf->bind();
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, (void *)0);
    glBindVertexArray(0);
    this->nvertices = 0;
}

void Renderer::push_triangle(Position positions[3],
                             Color colors[3]) {
    if ((this->nvertices + 3) > this->VERTEX_BUFFER_LEN) {
        printf("Vertex attribute buffers full, forcing draw\n");
        this->render_loop();
    }

    for (int i = 0; i < 3; i++) {
        this->pos_buf->set(this->nvertices, positions[i]);
        this->color_buf->set(this->nvertices, colors[i]);
        this->nvertices += 1;
    }
}

void Renderer::push_quad(Position positions[4],
                         Color colors[4]) {
    if ((this->nvertices + 6) > this->VERTEX_BUFFER_LEN) {
        this->render_loop();
    }
    for (int i = 0; i < 3; i++) {
        this->pos_buf->set(this->nvertices, positions[i]);
        this->color_buf->set(this->nvertices, colors[i]);
        this->nvertices += 1;
    }
    for (int i = 1; i < 4; i++) {
        this->pos_buf->set(this->nvertices, positions[i]);
        this->color_buf->set(this->nvertices, colors[i]);
        this->nvertices += 1;
    }
}

Renderer::~Renderer() {
    delete this->program;
    delete this->color_buf;
    delete this->pos_buf;

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::draw() {
    this->color_buf->flush();
    this->pos_buf->flush();

    this->program->use();
    glBindVertexArray(this->vao);
    glDrawArrays(GL_TRIANGLES, 0, nvertices);

    glFlush();
    nvertices = 0;
}
void Renderer::render_loop() {
    this->update();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwWindowShouldClose(window))
        this->~Renderer();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    this->draw();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Menu");
    ImGui::Text("Renderer: %s", this->renderer);
    ImGui::Text("Vendor: %s", this->vendor);
    ImGui::Text("FPS: %d", this->fps);
    ImGui::Text("Frame time: %f",
                ((float)1 / this->fps) * 1000.0f);
    ImGui::End();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::update() {
    float currentFrame = glfwGetTime();
    this->deltaTime = currentFrame - this->lastFrame;
    this->lastFrame = currentFrame;

    float elapsed_seconds = currentFrame - previousSecond;
    if (elapsed_seconds > 1.0f) {
        previousSecond = currentFrame;
        this->fps = (int)(frameCount / elapsed_seconds);

        frameCount = 0;
    }
    frameCount++;
}
