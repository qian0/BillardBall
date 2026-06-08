#pragma once

#include <glm/glm.hpp>

#include "Mesh.hpp"
#include "Shader.hpp"

// Owns all GPU resources for the table surface and four cushion walls.
struct TableScene
{
    Mesh surface;
    Mesh longCushion;  // shared by north and south cushions
    Mesh shortCushion; // shared by east and west cushions

    // Creates the table surface and cushion meshes from Table constants.
    static TableScene create();

    // Draws the table surface and four cushions using the flat shader.
    void draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
              const glm::vec3 &lightDir) const;

    // Releases all GPU resources.
    void destroy();
};
