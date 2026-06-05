#pragma once

#include "Texture.hpp"
#include <glm/glm.hpp>
#include <string>

// Generates a billiard-ball face texture: a solid colour background, a white
// circle in the centre, and a number rendered inside using stb_truetype.
struct BallTexture
{
    // Creates and returns a GPU texture for the given ball number and base colour.
    // fontPath: path to a TTF file used to rasterize the number.
    // size: texture resolution in pixels (square).
    static Texture generate(int number, glm::vec3 color, const std::string &fontPath, int size = 512);
};
