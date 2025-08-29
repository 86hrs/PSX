#pragma once
#include "glad.h"
#include "GLFW/glfw3.h"
#include "shader.h"
#include "gl_buffer.h"
#include "structs.h"

struct Renderer {
    Renderer();
    ~Renderer();
    void run(); 

    GLuint vao;
    uint32_t nvertices;

    // Buffer<Color> colors;
    // Buffer<Position> positions;
        
    GLFWwindow* window;
    Shader* program;
};
