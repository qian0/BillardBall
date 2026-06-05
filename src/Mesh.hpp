#pragma once

#include <glad/glad.h>

// Holds GPU buffers (VAO/VBO/EBO) for a piece of geometry and issues draw calls.
struct Mesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int indexCount;

    // Generates a UV sphere with the given number of latitude rings and longitude sectors.
    // Each vertex carries a position (location 0), a surface normal (location 1),
    // and a texture coordinate (location 2) in equirectangular UV space.
    static Mesh uvSphere(int rings, int sectors);

    // Draws the mesh using the currently bound shader program.
    void draw() const;

    // Releases all GPU buffer objects.
    void destroy();
};
