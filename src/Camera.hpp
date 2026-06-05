#pragma once

#include <glm/glm.hpp>

// Orbiting camera defined in spherical coordinates around a fixed target point.
// Mouse drag rotates the orbit; scroll wheel adjusts the distance.
struct Camera
{
    float radius;       // distance from target — controls how far the camera sits from the scene centre; scroll wheel changes this
    float theta;        // azimuth angle in radians — sweeps the camera left/right around the vertical axis; mouse drag X changes this
    float phi;          // elevation angle in radians — tilts the camera up/down; clamped to avoid flipping at the poles; mouse drag Y changes this
    glm::vec3 target;   // world-space point the camera always looks at — the centre of the table

    // Initialises the camera at the given spherical position looking at target.
    Camera(float radius, float theta, float phi, glm::vec3 target = {0.0f, 0.0f, 0.0f});

    // Returns the world-space position of the camera eye.
    glm::vec3 position() const;

    // Builds a view matrix from the current spherical coordinates.
    glm::mat4 viewMatrix() const;

    // Rotates the orbit by a mouse delta (pixels), scaled to radians.
    void onMouseDrag(float dx, float dy);

    // Zooms by adjusting the orbit radius.
    void onScroll(float delta);
};
