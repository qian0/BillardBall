# M4 — Basic Physics (linear only) — Detailed Plan

## Goal

Balls move, bounce off cushions and each other, and come to rest.
No spin, no rolling friction — linear velocity with a linear damping stub.

---

## New Files

| File | Purpose |
|------|---------|
| `src/Physics.hpp` | `BallState` struct + `Physics` class interface |
| `src/Physics.cpp` | `step()`, sphere-sphere, sphere-cushion, damping |
| `unit_tests/m4/m4_0_headless.cpp` | Headless collision correctness assertions (no GL) |
| `unit_tests/m4/m4_1_movingBalls.cpp` | Visual test: cue ball struck with initial velocity |

---

## Steps

### Step 1 — BallState struct (`src/Physics.hpp`)

```cpp
struct BallState
{
    glm::vec3 pos;
    glm::vec3 vel;
    float radius;
    float invMass; // 1/mass; set to 0 for static/pocketed balls
    bool pocketed;
};
```

All 16 balls have equal mass. `invMass = 1.0f` for all active balls.
Set `pocketed = false`; `vel = {0,0,0}` for the static rack.

---

### Step 2 — Physics class (`src/Physics.hpp`)

```cpp
class Physics
{
public:
    static constexpr float kDt          = 1.0f / 120.0f;
    static constexpr float kDamping     = 0.9975f;  // per-step; tune so a hard break travels ~full table
    static constexpr float kRestitution = 0.85f;    // applied to both ball-ball and ball-cushion

    std::array<BallState, 16> balls;

    // Seeds ball positions and radii from BallScene's rack layout.
    void initFromScene(const BallScene &scene);

    // Advances one fixed timestep: integrate → resolve cushions → resolve ball-ball.
    void step();

    // Returns true when every active ball's speed is below the rest threshold.
    bool allAtRest() const;
};
```

---

### Step 3 — Fixed-timestep accumulator in `main.cpp`

```cpp
double prevTime   = glfwGetTime();
double accumulator = 0.0;

while (!glfwWindowShouldClose(window)) {
    double now  = glfwGetTime();
    double dt   = now - prevTime;
    prevTime    = now;
    accumulator += dt;

    while (accumulator >= Physics::kDt) {
        physics.step();
        accumulator -= Physics::kDt;
    }

    // render with current positions — no interpolation needed for M4
    ballScene.draw(phong, view, projection, kLightDir, camPos, physics.balls);
}
```

Cap `dt` to `0.25f` seconds to prevent a spiral of death on focus-steal pauses.

---

### Step 4 — Euler integration (`Physics::step`)

```cpp
for (auto &b : balls) {
    if (b.pocketed || b.invMass == 0.0f) { continue; }
    b.pos += b.vel * kDt;
    b.vel *= kDamping;
}
```

Order: integrate first, then resolve collisions — prevents tunnelling through thin cushions.

---

### Step 5 — Cushion plane collision

Six half-spaces derived from `Table` constants. For each active ball, clamp and reflect:

```cpp
// X axis — long cushions
float xMin = -Table::kLength * 0.5f + b.radius;
float xMax =  Table::kLength * 0.5f - b.radius;
if (b.pos.x < xMin) { b.pos.x = xMin; b.vel.x = std::abs(b.vel.x) * kRestitution; }
if (b.pos.x > xMax) { b.pos.x = xMax; b.vel.x = -std::abs(b.vel.x) * kRestitution; }

// Z axis — short cushions
float zMin = -Table::kWidth * 0.5f + b.radius;
float zMax =  Table::kWidth * 0.5f - b.radius;
if (b.pos.z < zMin) { b.pos.z = zMin; b.vel.z = std::abs(b.vel.z) * kRestitution; }
if (b.pos.z > zMax) { b.pos.z = zMax; b.vel.z = -std::abs(b.vel.z) * kRestitution; }
```

Y is fixed: balls always sit at `pos.y = Table::kBallR` (no vertical physics in M4).

---

### Step 6 — Sphere-sphere collision

O(n²), n=16 — trivially fast. For every unique pair (i, j):

```cpp
glm::vec3 delta = balls[j].pos - balls[i].pos;
float dist      = glm::length(delta);
float minDist   = balls[i].radius + balls[j].radius;

if (dist < minDist && dist > 1e-6f) {
    glm::vec3 n = delta / dist;
    float overlap = minDist - dist;

    // Positional correction — push apart symmetrically
    balls[i].pos -= n * overlap * 0.5f;
    balls[j].pos += n * overlap * 0.5f;

    // Impulse response
    float relVel = glm::dot(balls[j].vel - balls[i].vel, n);
    if (relVel < 0.0f) {
        float j_imp = -(1.0f + kRestitution) * relVel
                    / (balls[i].invMass + balls[j].invMass);
        balls[i].vel -= n * j_imp * balls[i].invMass;
        balls[j].vel += n * j_imp * balls[j].invMass;
    }
}
```

Run cushion resolution before sphere-sphere each step to prevent balls escaping through cushions during a tight rack break.

---

### Step 7 — BallScene draw signature change

`BallScene::draw` currently derives position from the static `BallDef::pos`. For M4 we
pass physics positions at draw time without touching the stored rack layout:

```cpp
// New overload in BallScene.hpp:
void draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &proj,
          const glm::vec3 &lightDir, const glm::vec3 &camPos,
          const std::array<BallState, 16> &states) const;
```

The existing `draw()` (no states arg) keeps working for static M3-style tests.
Inside, replace `balls[i].pos` with `states[i].pos` when building the model matrix.

---

### Step 8 — `allAtRest()` utility

```cpp
static constexpr float kRestThreshold = 0.003f; // world units per second

bool Physics::allAtRest() const
{
    for (const auto &b : balls) {
        if (!b.pocketed && glm::length(b.vel) > kRestThreshold) { return false; }
    }
    return true;
}
```

Used in M5 to decide when the player may shoot again.

---

### Step 9 — Space-bar impulse in `main.cpp`

For M4 verification, pressing Space fires the cue ball toward the rack:

```cpp
// In onKey:
if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
    physics.balls[0].vel = glm::vec3(0.0f, 0.0f, -4.0f); // m/s toward rack
}
```

This keeps the scene interactive without needing M5's aiming system.

---

### Step 10 — `initFromScene` (`Physics::step`)

Seeds physics state from the rack so positions stay consistent:

```cpp
void Physics::initFromScene(const BallScene &scene)
{
    for (int i = 0; i < 16; ++i) {
        balls[i].pos      = scene.balls[i].pos;
        balls[i].vel      = glm::vec3(0.0f);
        balls[i].radius   = Table::kBallR;
        balls[i].invMass  = 1.0f;
        balls[i].pocketed = false;
    }
}
```

---

### Step 11 — Unit tests

**`m4_0_headless.cpp`** — no GL, just assert physics invariants:

| Scenario | Expected outcome |
|----------|-----------------|
| Two balls at `2R - 0.001f` apart, zero velocity | After `step()`: separated, both moving apart |
| Ball at left wall with `vel.x = -2.0f` | After `step()`: `vel.x > 0`, speed ≈ `2.0 * kRestitution` |
| All balls stationary | `allAtRest()` returns true |
| Cue ball moving at `0.001f/s` | `allAtRest()` returns false |

**`m4_1_movingBalls.cpp`** — visual test:

Clone of `m3_5_fullScene.cpp` with:
- `Physics` added; `initFromScene` called after `BallScene::create`
- Space bar fires the cue ball
- Draw loop updated to pass physics positions
- Window title: `"M4.1 — Moving Balls"`

Verify by inspection:
- Balls scatter from a break shot and come to rest
- No ball escapes through a cushion
- No permanent overlap between balls after the rack settles

---

## Key Decisions

| Decision | Reason |
|----------|--------|
| 120 Hz fixed timestep | Stable at billiards speeds; decoupled from vsync |
| Positional correction (push-apart) | Prevents compounding penetration in the tight rack |
| Cushion resolved before sphere-sphere | Avoids balls being pushed out of bounds by sphere impulse |
| `kDamping` per-step not per-second | Simpler; value chosen empirically so balls stop in ~5-8 s |
| Y fixed at `kBallR` | No vertical physics needed until pockets (M7) |
| Static rack layout kept in `BallScene` | Physics reads positions; rendering reads physics; no coupling |
| Existing `draw()` overload preserved | M3 unit tests continue to compile unchanged |
