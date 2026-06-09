#include "Physics.hpp"

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Table.hpp"

// Advances one fixed timestep: integrate → resolve cushions → resolve sphere-sphere pairs.
void Physics::step()
{
    // Euler integration + per-step damping + rolling rotation
    for (auto &b : balls) {
        if (b.pocketed || b.invMass == 0.0f) { continue; }
        b.pos += b.vel * kDt;
        b.vel *= kDamping;
        b.pos.y = Table::kBallR; // no vertical physics in M4

        // Rolling constraint: axis perpendicular to velocity in the horizontal plane.
        // cross(vel_dir, up) gives the axis so that the top of the ball moves forward.
        glm::vec3 velXZ(b.vel.x, 0.0f, b.vel.z);
        float speed = glm::length(velXZ);
        if (speed > 1e-6f) {
            glm::vec3 rollAxis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), velXZ / speed);
            float angle = speed * kDt / b.radius;
            b.rotation = glm::normalize(glm::angleAxis(angle, rollAxis) * b.rotation);
        }
    }

    // Cushion plane reflections — resolved before sphere-sphere so balls cannot be
    // pushed out of bounds by a collision impulse applied in the same step.
    for (auto &b : balls) {
        if (b.pocketed || b.invMass == 0.0f) { continue; }

        const float xMin = -Table::kLength * 0.5f + b.radius;
        const float xMax =  Table::kLength * 0.5f - b.radius;
        if (b.pos.x < xMin) { b.pos.x = xMin; b.vel.x =  std::abs(b.vel.x) * kRestitution; }
        if (b.pos.x > xMax) { b.pos.x = xMax; b.vel.x = -std::abs(b.vel.x) * kRestitution; }

        const float zMin = -Table::kWidth * 0.5f + b.radius;
        const float zMax =  Table::kWidth * 0.5f - b.radius;
        if (b.pos.z < zMin) { b.pos.z = zMin; b.vel.z =  std::abs(b.vel.z) * kRestitution; }
        if (b.pos.z > zMax) { b.pos.z = zMax; b.vel.z = -std::abs(b.vel.z) * kRestitution; }
    }

    // Sphere-sphere impulse + positional correction for every unique active pair.
    for (int i = 0; i < 16; ++i) {
        if (balls[i].pocketed || balls[i].invMass == 0.0f) { continue; }
        for (int j = i + 1; j < 16; ++j) {
            if (balls[j].pocketed || balls[j].invMass == 0.0f) { continue; }

            glm::vec3 delta = balls[j].pos - balls[i].pos;
            float dist      = glm::length(delta);
            float minDist   = balls[i].radius + balls[j].radius;

            if (dist >= minDist || dist < 1e-6f) { continue; }

            glm::vec3 n   = delta / dist;
            float overlap = minDist - dist;

            // Push apart symmetrically to prevent compounding penetration
            balls[i].pos -= n * overlap * 0.5f;
            balls[j].pos += n * overlap * 0.5f;

            float relVel = glm::dot(balls[j].vel - balls[i].vel, n);
            if (relVel >= 0.0f) { continue; } // already separating

            float j_imp = -(1.0f + kRestitution) * relVel
                        / (balls[i].invMass + balls[j].invMass);
            balls[i].vel -= n * j_imp * balls[i].invMass;
            balls[j].vel += n * j_imp * balls[j].invMass;
        }
    }
}

// Returns true when every active ball's speed is below kRestThreshold.
bool Physics::allAtRest() const
{
    for (const auto &b : balls) {
        if (!b.pocketed && glm::length(b.vel) > kRestThreshold) { return false; }
    }
    return true;
}
