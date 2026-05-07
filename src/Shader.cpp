#include "Shader.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << "\n";
        return "";
    }
    std::string result, line;
    std::string dir = path.substr(0, path.find_last_of("/\\") + 1);
    while (std::getline(file, line)) {
        if (line.rfind("#include \"", 0) == 0) {
            auto start = line.find('"') + 1;
            auto end   = line.rfind('"');
            if (start < end)
                result += read_file(dir + line.substr(start, end - start)) + '\n';
        } else {
            result += line + '\n';
        }
    }
    return result;
}

static unsigned int compile_shader(GLenum type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compilation error:\n" << log << "\n";
    }
    return shader;
}

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) {
    unsigned int vert = compile_shader(GL_VERTEX_SHADER,   read_file(vertex_path));
    unsigned int frag = compile_shader(GL_FRAGMENT_SHADER, read_file(fragment_path));

    program_id = glCreateProgram();
    glAttachShader(program_id, vert);
    glAttachShader(program_id, frag);
    glLinkProgram(program_id);

    int success;
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program_id, 512, nullptr, log);
        std::cerr << "Shader linking error:\n" << log << "\n";
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    glDeleteProgram(program_id);
}

void Shader::use() const {
    glUseProgram(program_id);
}

void Shader::set_int(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
}

void Shader::set_float(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
}

void Shader::set_vec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::set_vec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::set_mat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
