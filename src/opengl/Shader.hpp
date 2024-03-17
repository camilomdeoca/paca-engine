#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    ~Shader();

    void bind();
    void unbind();

    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, const float &value);
    void setFloat2(const std::string &name, const glm::vec2 &value);
    void setFloat3(const std::string &name, const glm::vec3 &value);
    void setFloat4(const std::string &name, const glm::vec4 &value);
    void setMat3(const std::string &name, const glm::mat3 &matrix);
    void setMat4(const std::string &name, const glm::mat4 &matrix);

private:
    uint32_t m_id;
};
