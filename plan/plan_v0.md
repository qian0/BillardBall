# Billiard Ball 3D — Plan v0

## 1. Architecture

### Platform & Build
- **OS**: Linux
- **Build system**: CMake (3.20+)
- **C++ standard**: C++17

### Rendering
- **OpenGL 3.3 core profile** — wide driver support on Linux
- **GLAD** — already vendored; handles function pointer loading
- **GLFW** — window creation, OpenGL context, keyboard/mouse input
- **GLM** — header-only math library (vectors, matrices, quaternions)
- Sphere geometry generated procedurally (UV sphere, no asset loader needed at this stage)
- Shading: Phong model with a single directional light; specular highlight makes balls look convincing

### Physics (hand-rolled, Bullet-ready interface)
- Physics is isolated behind a `Physics` module so it can be swapped for Bullet later without touching rendering or game logic
- Ball state: `glm::vec3 position`, `glm::vec3 velocity`, `glm::vec3 angularVelocity`, `float radius`, `float mass`
- Integration: semi-implicit Euler (good enough for billiards timescales)
- Collision detection: sphere-sphere (exact), sphere-AABB cushion planes (analytic)
- Collision response: impulse-based with restitution coefficient
- Friction/spin: linear damping stub initially; proper rolling-with-slip transition in a later milestone
- Fixed timestep loop (e.g. 120 Hz physics, decoupled from render frame rate)

### Game Layer
- `Table`: defines play surface dimensions and six cushion planes
- `Ball`: data struct + unique ID (for numbering/texturing later)
- `GameState`: owns the ball array, drives the physics step, tracks turn state (aiming / in-motion / potted)
- Camera: simple orbiting camera controlled by mouse drag; fixed above-table perspective for now

### File Layout
```
BillardBall/
├── CMakeLists.txt
├── glad/                    # vendored
├── plan/
│   └── plan_v0.md
├── src/
│   ├── main.cpp             # entry point, game loop
│   ├── Renderer.hpp/cpp     # VAO/VBO setup, draw calls
│   ├── Shader.hpp/cpp       # compile/link GLSL programs, uniform helpers
│   ├── Ball.hpp             # Ball struct
│   ├── Physics.hpp/cpp      # step(), resolve collisions
│   ├── Table.hpp            # table geometry and cushion planes
│   └── GameState.hpp/cpp    # ball array, turn logic
└── shaders/
    ├── ball.vert
    └── ball.frag
```

---

## 2. Milestones

### M1 — Window + Triangle (build baseline)
**Goal**: prove the toolchain works end-to-end.
- CMakeLists.txt: find GLFW, link GLAD, pull GLM via FetchContent
- Open a GLFW window with an OpenGL 3.3 core context
- Clear to a background colour; draw a hard-coded triangle with a minimal shader pair
- **Test**: window opens, triangle visible, clean exit on Esc

### M2 — Sphere Renderer
**Goal**: render a single lit sphere on screen.
- Procedural UV sphere mesh (configurable rings/sectors)
- Phong vertex + fragment shaders (ambient + diffuse + specular)
- Orbiting camera controlled by mouse drag (arcball or spherical coords)
- **Test**: sphere visible, shading changes as camera orbits, no z-fighting artefacts

### M3 — Table & Static Scene
**Goal**: render the full static scene.
- Flat table quad with a felt-green colour/texture
- Cushion walls rendered as thin boxes or coloured quads
- 16 balls placed in standard rack formation (one cue ball, 15 object balls)
- **Test**: all balls visible in rack, camera can orbit the full table, correct relative scale

### M4 — Basic Physics (linear only)
**Goal**: balls move and bounce off each other and cushions.
- Fixed-timestep physics loop (120 Hz), decoupled from render
- Euler integration: position += velocity * dt
- Sphere-sphere collision detection + impulse response (no spin yet)
- Sphere-cushion AABB reflection
- Linear velocity damping (stub for friction)
- **Test**: apply an impulse to the cue ball, balls scatter, come to rest; no tunnelling at normal speeds

### M5 — Cue Stick Input
**Goal**: player can aim and strike.
- Aiming mode: mouse controls strike direction projected onto table plane
- On-screen cue stick line or arrow showing aim direction
- Click (or spacebar) applies an impulse to the cue ball proportional to a power value
- **Test**: full shot cycle — aim, strike, balls move, come to rest, can shoot again

### M6 — Rolling Friction & Spin
**Goal**: physically plausible ball motion.
- Rolling constraint: couple angular velocity to linear velocity once sliding stops
- Sliding → rolling transition (friction impulse each step)
- Spin transfer on ball-ball collisions (partial, simplified)
- **Test**: cue ball decelerates smoothly with correct rolling distance; top-spin / back-spin has a visible effect on final position

### M7 — Basic Game Rules
**Goal**: a playable 8-ball game loop.
- Pocket detection (six circular regions at table corners and midpoints)
- Ball removal on pocket
- Turn alternation: solids vs stripes assignment after first pot
- Win condition: 8-ball potted after clearing your group
- HUD: minimal text overlay (whose turn, ball group)
- **Test**: full game can be played to completion

### M8 — Visual Polish (stretch)
- Numbered ball textures (procedural or image-based via stb_image)
- Shadow map or fake blob shadow under each ball
- Table wood-grain texture on cushions/rails
- Sound effects via miniaudio (ball-ball, ball-cushion, pocket)

---

## Notes & Deferred Decisions
- **Bullet migration**: Physics.hpp/cpp exposes `void step(float dt)` and a `CollisionResult` type; replacing internals with Bullet should not require changes outside that module.
- **Networking / multiplayer**: not in scope for v0.
- **Mobile / other platforms**: Linux-only for now; GLFW and GLM are cross-platform if needed later.
