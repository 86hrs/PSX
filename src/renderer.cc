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
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

  this->window = glfwCreateWindow(800, 600, "PSX by *^hrs", NULL, NULL);
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

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  Buffer<Position> positions; 
  glEnableVertexAttribArray(0);
  glVertexAttribIPointer(0, 2, GL_SHORT, 0, nullptr);
  Buffer<Color> colors;
  glEnableVertexAttribArray(1);
  glVertexAttribIPointer(1, 3, GL_UNSIGNED_BYTE, 0, nullptr);
  this->run();
}

Renderer::~Renderer() {
  delete this->program;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void Renderer::run() {
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
