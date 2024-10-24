#include "opengl/Shader.hpp"

#include "utils/Assert.hpp"
#include "utils/Log.hpp"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iosfwd>
#include <vector>

std::string readFile(const std::string &path)
{
    std::string result;
    
    std::ifstream file(path, std::ios::binary);
    if (file)
    {
        file.seekg(0, std::ios::end);
        std::streampos size = file.tellg();
        if (size != -1)
        {
            result.resize(size);
            file.seekg(0, std::ios::beg);
            file.read(result.data(), size);
        }
    }
    else
        WARN("Tried to load file {} that does not exist", path);
    return result;
}

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath, std::list<ShaderCompileTimeParameter> parameters)
{
    // Read our shaders into the appropriate buffers
    std::string vertexSource = readFile(vertexPath); // Get source code for vertex shader.
    std::string fragmentSource = readFile(fragmentPath); // Get source code for fragment shader.
    
    // Set parameters (#define)
    std::string parametersAll;
    for (const ShaderCompileTimeParameter &param : parameters)
    {
        parametersAll += "#define " + param.value + "\n";
    }
    size_t secondLinePosVertex = vertexSource.find('\n') + 1;
    size_t secondLinePosFragment = fragmentSource.find('\n') + 1;
    vertexSource.insert(secondLinePosVertex, parametersAll);
    fragmentSource.insert(secondLinePosFragment, parametersAll);
    
    // Create an empty vertex shader handle
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    // Send the vertex shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    const GLchar *source = (const GLchar *)vertexSource.c_str();
    glShaderSource(vertexShader, 1, &source, 0);
    
    // Compile the vertex shader
    glCompileShader(vertexShader);
    
    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
    	GLint maxLength = 0;
    	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
    
    	// The maxLength includes the NULL character
    	std::vector<GLchar> infoLog(maxLength);
    	glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
    	
    	// We don't need the shader anymore.
    	glDeleteShader(vertexShader);
    
    	// Use the infoLog as you see fit.
        ERROR("{}", infoLog.data());
        ERROR("Error compiling vertex shader!: {}.", vertexPath);
        ASSERT(false);
    	
    	// In this simple program, we'll just leave
    	return;
    }
    
    // Create an empty fragment shader handle
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Send the fragment shader source code to GL
    // Note that std::string's .c_str is NULL character terminated.
    source = (const GLchar *)fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &source, 0);
    
    // Compile the fragment shader
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
    	GLint maxLength = 0;
    	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
    
    	// The maxLength includes the NULL character
    	std::vector<GLchar> infoLog(maxLength);
    	glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
    	
    	// We don't need the shader anymore.
    	glDeleteShader(fragmentShader);
    	// Either of them. Don't leak shaders.
    	glDeleteShader(vertexShader);
    
    	// Use the infoLog as you see fit.
        ERROR("{}", infoLog.data());
        ERROR("Error compiling fragment shader!: {}.", fragmentPath);
        ASSERT(false);
    	
    	// In this simple program, we'll just leave
    	return;
    }
    
    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    m_id = glCreateProgram();
    
    // Attach our shaders to our program
    glAttachShader(m_id, vertexShader);
    glAttachShader(m_id, fragmentShader);
    
    // Link our program
    glLinkProgram(m_id);
    
    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
    	GLint maxLength = 0;
    	glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &maxLength);
    
    	// The maxLength includes the NULL character
    	std::vector<GLchar> infoLog(maxLength);
    	glGetProgramInfoLog(m_id, maxLength, &maxLength, &infoLog[0]);
    	
    	// We don't need the program anymore.
    	glDeleteProgram(m_id);
    	// Don't leak shaders either.
    	glDeleteShader(vertexShader);
    	glDeleteShader(fragmentShader);
    
    	// Use the infoLog as you see fit.
        ERROR("{}", infoLog.data());
        ERROR("Error linking shader program!");
        ASSERT(false);
    	
    	// In this simple program, we'll just leave
    	return;
    }
    
    // Always detach shaders after a successful link.
    glDetachShader(m_id, vertexShader);
    glDetachShader(m_id, fragmentShader);
}

Shader::~Shader()
{
    glDeleteProgram(m_id);
}

void Shader::bind()
{
    glUseProgram(m_id);
}

void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::setInt(const std::string &name, int value)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1i(location, value);
}

void Shader::setUint(const std::string &name, unsigned int value)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1ui(location, value);
}

void Shader::setFloat(const std::string &name, const float &value)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform1f(location, value);
}

void Shader::setFloat2(const std::string &name, const glm::vec2 &value)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void Shader::setFloat3(const std::string &name, const glm::vec3 &value)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::setFloat4(const std::string &name, const glm::vec4 &value)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &matrix)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setMat4(const std::string &name, const glm::mat4 &matrix)
{
    GLint location = glGetUniformLocation(m_id, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

