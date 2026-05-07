#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertex_path, const std::string& fragment_path);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void set_int  (const std::string& name, int               value) const;
    void set_float(const std::string& name, float             value) const;
    void set_vec2 (const std::string& name, const glm::vec2&  value) const;
    void set_vec3 (const std::string& name, const glm::vec3&  value) const;
    void set_mat4 (const std::string& name, const glm::mat4&  value) const;

private:
    unsigned int program_id;
};
