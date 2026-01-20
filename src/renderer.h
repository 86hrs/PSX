#pragma once
#include "gl_buffer.h"
#include "GLFW/glfw3.h"
#include "glad.h"
#include "shader.h"
#include "structs.h"

struct Renderer {
    Renderer();
    ~Renderer();
    void render_loop();
    void update();

    static const uint32_t VERTEX_BUFFER_LEN = 64 * 1024;

    GLuint vao;
    uint32_t nvertices;

    Buffer<Color> *color_buf;
    Buffer<Position> *pos_buf;

    void push_triangle(Position positions[3], Color color[3]);
    void push_quad(Position position[4], Color color[4]);
    void draw();

    GLFWwindow *window;
    Shader *program;
    
    const char *vendor;
    const char *renderer;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float previousSecond = glfwGetTime();
    int frameCount = 0;
    int fps = 0;
};
