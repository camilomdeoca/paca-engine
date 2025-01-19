#pragma once

#include <utils/Log.hpp>

#include <format>
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
    Shader();
    Shader(
        const std::string &vertexPath,
        const std::string &fragmentPath,
        std::list<ShaderCompileTimeParameter> parameters = {});
    ~Shader();

    void init(
        const std::string &vertexPath,
        const std::string &fragmentPath,
        std::list<ShaderCompileTimeParameter> parameters = {});

    void bind();
    void unbind();

    template<typename T, class ...Args>
    void setUniform(T value, std::format_string<Args...> fmt, Args &&...args)
    {
        static char buf[128] {0};

        const std::format_to_n_result result
            = std::format_to_n(buf, sizeof(buf) - 1, fmt, std::forward<Args>(args)...);

        // *result.out = '\0'; // can't do this because the std library is incorrectly implemented
        size_t textLength = static_cast<size_t>(result.size) < sizeof(buf) - 1
            ? result.size
            : sizeof(buf) - 1;
        buf[textLength] = '\0';

        setUniform(buf, value);
    }

    //void awda()
    //{
    //    static char buf[128] {0};

    //    const std::format_to_n_result result
    //        = std::format_to_n(buf, sizeof(buf) - 1, "Holaaa {}", 12);

    //    // *result.out = '\0'; // can't do this because the std library is incorrectly implemented
    //    size_t textLength = result.size < sizeof(buf) - 1 ? result.size : sizeof(buf) - 1;
    //    buf[textLength] = '\0';
    //}

private:
    void setUniform(const char *name, int value);
    void setUniform(const char *name, unsigned int value);
    void setUniform(const char *name, const float &value);
    void setUniform(const char *name, const glm::vec2 &value);
    void setUniform(const char *name, const glm::vec3 &value);
    void setUniform(const char *name, const glm::vec4 &value);
    void setUniform(const char *name, const glm::mat3 &matrix);
    void setUniform(const char *name, const glm::mat4 &matrix);

    uint32_t m_id;
};
