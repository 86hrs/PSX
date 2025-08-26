#pragma once
#include "glad.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Shader {
  public:
    GLuint ID;

    Shader(const char *vertexPath, const char *fragmentPath);
    ~Shader();

    void use();
    static void stop() { glUseProgram(0); }
    void check_compile_errors(GLuint shader, std::string type);
    void set_bool(const std::string &name, bool value) const;
    void set_int(const std::string &name, int value) const;
    void set_float(const std::string &name, float value) const;
    void set_mat4(const std::string &name, const glm::mat4 &mat) const;
    void set_vec3(const std::string &name, const glm::vec3 &value) const;
    void set_int_array(const std::string &name,
                       const std::vector<int> &values) const;
};
