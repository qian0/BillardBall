#pragma once

#include <glm/glm.hpp>
#include <string>

#include "Mesh.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

// Position, colour, and number label for a single ball.
struct BallDef
{
    int number;
    glm::vec3 color;
    glm::vec3 pos;
};

// Owns all GPU resources for the 16 billiard balls (sphere mesh, textures, positions).
struct BallScene
{
    Mesh sphere;
    Texture textures[16];
    BallDef balls[16];

    // Creates the sphere mesh, generates 16 ball textures, and places balls in rack formation.
    static BallScene create(const std::string &fontPath);

    // Draws all 16 balls using the Phong shader with decal projection.
    void draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
              const glm::vec3 &lightDir, const glm::vec3 &cameraPos) const;

    // Releases all GPU resources.
    void destroy();
};
