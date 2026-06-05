#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

// Slurps an entire text file into a string; throws if the path cannot be opened.
static std::string readFile(const char *path)
{
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error(std::string("Shader::readFile: cannot open ") + path);
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Compiles one shader stage (vertex or fragment) from source on disk.
// Returns the GL shader handle on success; throws a descriptive error on failure.
GLuint Shader::compile(const char *path, GLenum type)
{
    std::string src = readFile(path);
    const char *csrc = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader compile error (") + path + "): " + log);
    }
    return shader;
}

// Compiles both stages, links them into a program, then frees the stage objects
// (they are no longer needed once the program is linked).
Shader::Shader(const char *vertPath, const char *fragPath)
{
    GLuint vert = compile(vertPath, GL_VERTEX_SHADER);
    GLuint frag = compile(fragPath, GL_FRAGMENT_SHADER);

    id = glCreateProgram();
    glAttachShader(id, vert);
    glAttachShader(id, frag);
    glLinkProgram(id);

    GLint ok;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        glDeleteProgram(id);
        glDeleteShader(vert);
        glDeleteShader(frag);
        throw std::runtime_error(std::string("Shader link error: ") + log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

// Frees the GPU program; safe to call even if the program was never successfully linked.
Shader::~Shader()
{
    glDeleteProgram(id);
}

void Shader::use() const
{
    glUseProgram(id);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

// Uploads a 3x3 matrix uniform (e.g. the normal matrix for correct lighting after non-uniform scale).
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const
{
    glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

// Uploads a scalar int uniform — required for sampler2D uniforms which expect an int texture unit index.
void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}
