#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

// Owns a linked OpenGL shader program built from a vertex/fragment source pair.
// Provides typed uniform setters so callers never touch raw uniform locations.
class Shader
{
public:
    GLuint id;

    // Reads GLSL source from disk, compiles both stages, and links the program.
    Shader(const char *vertPath, const char *fragPath);

    // Releases the GPU program object.
    ~Shader();

    // Installs this program as the active pipeline for subsequent draw calls.
    void use() const;

    // Uploads a 4x4 matrix uniform (e.g. model/view/projection transforms).
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

    // Uploads a 3x3 matrix uniform (e.g. the normal matrix for lighting).
    void setMat3(const std::string &name, const glm::mat3 &mat) const;

    // Uploads a 3-component float vector uniform (e.g. colours, positions, normals).
    void setVec3(const std::string &name, const glm::vec3 &vec) const;

    // Uploads a scalar float uniform (e.g. shininess, time, blend factors).
    void setFloat(const std::string &name, float value) const;

    // Uploads a scalar int uniform (e.g. texture unit indices for sampler2D uniforms).
    void setInt(const std::string &name, int value) const;

private:
    // Compiles a single shader stage from a file path; throws on error.
    static GLuint compile(const char *path, GLenum type);
};
