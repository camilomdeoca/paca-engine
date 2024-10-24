#pragma once

#include <glm/glm.hpp>
#include <list>
#include <string>

struct ShaderCompileTimeParameter {
    ShaderCompileTimeParameter(const std::string &name)
        : value(name) {}

    ShaderCompileTimeParameter(const std::string &name, const std::string &value)
        : value(name + " " + value) {}

    template<typename T>
    ShaderCompileTimeParameter(const std::string &name, T value)
        : value(name + " " + std::to_string(value)) {}

    std::string value;
};

class Shader {
public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath, std::list<ShaderCompileTimeParameter> parameters = {});
    ~Shader();

    void bind();
    void unbind();

    void setInt(const std::string &name, int value);
    void setUint(const std::string &name, unsigned int value);
    void setFloat(const std::string &name, const float &value);
    void setFloat2(const std::string &name, const glm::vec2 &value);
    void setFloat3(const std::string &name, const glm::vec3 &value);
    void setFloat4(const std::string &name, const glm::vec4 &value);
    void setMat3(const std::string &name, const glm::mat3 &matrix);
    void setMat4(const std::string &name, const glm::mat4 &matrix);

private:
    uint32_t m_id;
};
