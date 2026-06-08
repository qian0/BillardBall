#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

// Sets the initial spherical coordinates and the point the camera orbits.
Camera::Camera(float radius, float theta, float phi, glm::vec3 target)
    : radius{radius}, theta{theta}, phi{phi}, target{target}
{
}

// Converts spherical coordinates to a Cartesian eye position in world space.
glm::vec3 Camera::position() const
{
    return target + glm::vec3{
        radius * std::cos(phi) * std::cos(theta),
        radius * std::sin(phi),
        radius * std::cos(phi) * std::sin(theta)
    };
}

// Builds a right-handed look-at view matrix from the current eye position toward the target.
glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(position(), target, glm::vec3{0.0f, 1.0f, 0.0f});
}

// Converts pixel mouse deltas to radian angle changes and updates the orbit angles.
void Camera::onMouseDrag(float dx, float dy)
{
    const float sensitivity = 0.005f;
    theta += dx * sensitivity;
    phi   += dy * sensitivity;
    // Clamp elevation so the camera never flips over the poles
    phi = std::clamp(phi, 0.05f, 1.55f);
}

// Translates the orbit target along the camera's right and up axes so the scene pans with the cursor.
// Sensitivity scales with radius so panning feels consistent at any zoom level.
void Camera::onMousePan(float dx, float dy)
{
    const float sensitivity = 0.001f * radius;
    glm::vec3 forward = glm::normalize(target - position());
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3{0.0f, 1.0f, 0.0f}));
    glm::vec3 up = glm::cross(right, forward);
    target -= right * (dx * sensitivity);
    target += up * (dy * sensitivity);
}

// Shrinks or grows the orbit radius so the user can dolly in and out.
void Camera::onScroll(float delta)
{
    const float speed = 0.5f;
    radius -= delta * speed;
    radius = std::max(radius, 1.0f);
}
