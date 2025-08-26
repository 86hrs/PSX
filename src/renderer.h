#pragma once
#include "glad.h"
#include "GLFW/glfw3.h"
#include "shader.h"

struct Renderer {
    Renderer();
    ~Renderer();
    void run(); 
    
    GLFWwindow* window;
    Shader* program;
};
