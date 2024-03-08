#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    ~Shader();

    void bind();
    void unbind();

    void setFloat3(const std::string &name, const glm::vec3 &value);
    void setFloat4(const std::string &name, const glm::vec4 &value);
    void setMat4(const std::string &name, const glm::mat4 &matrix);

private:
    uint32_t m_id;
};
