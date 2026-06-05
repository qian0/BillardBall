#pragma once

#include <glad/glad.h>

// Owns a single 2D OpenGL texture object uploaded from a CPU-side RGBA bitmap.
struct Texture
{
    GLuint id;

    // Uploads an RGBA bitmap (width × height × 4 bytes) to the GPU and sets linear filtering.
    static Texture fromRGBA(const unsigned char *pixels, int width, int height);

    // Binds the texture to the given texture unit so shaders can sample it.
    void bind(int unit = 0) const;

    // Releases the GPU texture object.
    void destroy();
};
