# BillardBall — Progress

_Last updated: 2026-06-07_

---

## Milestones

| # | Name | Status |
|---|------|--------|
| M1 | Window + Triangle | Done |
| M2 | Sphere Renderer + Numbered Ball Texture | Done |
| M3 | Table & Static Scene | In progress |
| M4 | Basic Physics (linear only) | Not started |
| M5 | Cue Stick Input | Not started |
| M6 | Rolling Friction & Spin | Not started |
| M7 | Basic Game Rules | Not started |
| M8 | Visual Polish | Not started |

---

## M1 — Window + Triangle (Done)

**What was built:**
- CMakeLists.txt wired up: GLAD (vendored), GLFW (system), GLM (FetchContent 1.0.1)
- GLFW window with OpenGL 3.3 core context, vsync, Esc-to-quit
- Minimal vertex/fragment shader pair compiles and renders a hard-coded triangle
- `Shader` class: loads GLSL from disk, compiles both stages, links program, typed uniform setters

**Infrastructure added alongside M1:**
- `plan/` folder with `plan_v0.md` (architecture + milestone list), `system.txt`, `system_detect.sh`
- `.vscode/c_cpp_properties.json` pointing at `build/compile_commands.json` for IntelliSense
- `CMAKE_EXPORT_COMPILE_COMMANDS ON` baked into CMakeLists.txt
- `util.hpp`: `chdirToExe()` — changes working directory to binary location so relative asset paths resolve regardless of launch directory
- `unit_tests/` folder with its own `CMakeLists.txt`; test targets built with `-DBUILD_TESTS=ON`
- `unit_tests/m1_triangle.cpp`: standalone M1 regression test with own shaders

**Key decisions:**
- `unit_tests/` has its own `CMakeLists.txt` included via `add_subdirectory`; game and tests build independently
- `add_unit_test(name)` CMake helper function accepts extra source files via `${ARGN}`

---

## M2 — Sphere Renderer + Numbered Ball Texture (Done)

**What was built:**
- `Mesh` struct: VAO/VBO/EBO wrapper; `uvSphere(rings, sectors)` generates a unit sphere with position (loc 0), normal (loc 1), UV texcoord (loc 2)
- `Camera` struct: spherical-coordinate orbiting camera; mouse drag orbits (theta/phi), scroll wheel zooms
- Phong shading (`ball.vert` / `ball.frag`): ambient + diffuse + specular with a single directional light
- `Texture` struct: uploads RGBA bitmap to GPU, generates mipmaps, typed `bind(unit)` / `destroy()`
- `BallTexture`: generates a numbered billiard ball texture at runtime using `stb_truetype`
- `stb_truetype.h` vendored in `third_party/stb/`
- `DejaVuSans-Bold.ttf` in `assets/fonts/`, deployed next to binary at build time
- `unit_tests/m2_ball.cpp`: standalone M2 regression test with frozen shaders in `unit_tests/shaders/`

**Key decisions:**
- Switched from equirectangular UV texture mapping to **decal projection** for the ball number disc. Equirectangular wrapping distorts a circular disc into an oval; decal projection computes UV from the world-space normal (`decalUV = N.xy + 0.5`), giving a geometrically round disc.
- Number centering uses `stbtt_GetCodepointBitmapBox` to find the tight pixel bounding box rather than the full line-height box — digits have no descenders so line-height centering pushed numbers off-center.
- BallTexture background is transparent (alpha=0); ball colour is supplied via `uBallColor` uniform.
- `uUseDecal` bool uniform added to `ball.frag` so non-ball surfaces (table, cushions) don't accidentally sample the ball texture on faces where `N.z > 0`.

---

## M3 — Table & Static Scene (In progress)

**Detailed plan:** `plan/plan_m3_details.md`

**What remains:**
- `Mesh::quad` and `Mesh::box` implementations in `Mesh.cpp`
- `Table.hpp` with scene constants
- `ball.frag` updated with `uUseDecal` uniform
- `BallTexture.cpp` updated to skip disc/number for ball 0 (cue ball)
- `main.cpp` rewritten for the full M3 scene

---

## Source File Index

| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point, game loop, GLFW callbacks |
| `src/Shader.hpp/cpp` | GLSL program wrapper + uniform setters |
| `src/Mesh.hpp/cpp` | VAO/VBO/EBO + UV sphere, quad, box generators |
| `src/Camera.hpp/cpp` | Spherical-coordinate orbiting camera |
| `src/Texture.hpp/cpp` | 2D GL texture upload and bind |
| `src/BallTexture.hpp/cpp` | Runtime numbered ball texture via stb_truetype |
| `src/Table.hpp` | Table dimension constants |
| `src/util.hpp` | `chdirToExe()` — portable asset path helper |
| `shaders/ball.vert` | Sphere vertex shader (MVP + normal passthrough) |
| `shaders/ball.frag` | Phong lighting + decal projection |
| `unit_tests/m1_triangle.cpp` | M1 standalone regression test |
| `unit_tests/m2_ball.cpp` | M2 standalone regression test (frozen shaders) |
| `unit_tests/shaders/` | Shaders used exclusively by unit tests |
| `third_party/stb/stb_truetype.h` | Vendored font rasterizer |
| `assets/fonts/DejaVuSans-Bold.ttf` | Font used for ball number labels |
| `plan/plan_v0.md` | Full architecture doc and milestone definitions |
| `plan/plan_m3_details.md` | Detailed M3 implementation plan |
| `plan/system.txt` | Auto-generated system/library version report |
| `plan/system_detect.sh` | Script that regenerates system.txt |
