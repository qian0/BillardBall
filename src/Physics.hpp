#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// State for a single billiard ball tracked by the physics engine.
struct BallState
{
    glm::vec3 pos;
    glm::vec3 vel;
    glm::quat rotation; // accumulated rolling rotation — identity at rest
    float radius;
    float invMass; // 1/mass; 0 for static or pocketed balls
    bool pocketed;
};

// Fixed-timestep rigid-body physics for the 16 billiard balls.
// Linear velocities only — no spin or rolling friction (M6).
class Physics
{
public:
    static constexpr float kDt           = 1.0f / 120.0f;
    static constexpr float kDamping      = 0.9975f; // per-step; ball travels ~full table before rest
    static constexpr float kRestitution  = 0.85f;   // ball-ball and ball-cushion
    static constexpr float kRestThreshold = 0.003f; // speed below which a ball is considered at rest

    std::array<BallState, 16> balls;

    // Advances one fixed timestep: integrate → resolve cushions → resolve ball-ball pairs.
    void step();

    // Returns true when every active ball's speed is below kRestThreshold.
    bool allAtRest() const;
};
