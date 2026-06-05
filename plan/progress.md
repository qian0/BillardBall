# BillardBall — Progress

_Last updated: 2026-06-04_

---

## Milestones

| # | Name | Status |
|---|------|--------|
| M1 | Window + Triangle | Done |
| M2 | Sphere Renderer + Numbered Ball Texture | Done |
| M3 | Table & Static Scene | Not started |
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
- `add_unit_test(name)` CMake helper function keeps per-test boilerplate minimal

---

## M2 — Sphere Renderer + Numbered Ball Texture (Done)

**What was built:**
- `Mesh` struct: VAO/VBO/EBO wrapper; `uvSphere(rings, sectors)` generates a unit sphere with position (loc 0), normal (loc 1), UV texcoord (loc 2)
- `Camera` struct: spherical-coordinate orbiting camera; mouse drag orbits (theta/phi), scroll wheel zooms
- Phong shading (`ball.vert` / `ball.frag`): ambient + diffuse + specular with a single directional light; texture sampled and blended with base colour
- `Texture` struct: uploads RGBA bitmap to GPU, generates mipmaps, typed `bind(unit)` / `destroy()`
- `BallTexture`: generates a numbered billiard ball texture at runtime using `stb_truetype`
  - Solid colour background → white disc → black number rasterized with DejaVu Sans Bold
- `stb_truetype.h` vendored in `third_party/stb/`
- `DejaVuSans-Bold.ttf` copied to `assets/fonts/` and deployed next to binary at build time
- `Shader` extended: `setMat3`, `setInt` added

**Current visual result:**
- Black ball (number 8) rendered with Phong shading and a white disc + "8" label on its surface
- Left-drag orbits the camera; scroll zooms

**Key decisions:**
- UV sphere uses equirectangular mapping (u = longitude / 2π, v = latitude / π); seam handled by duplicating the s=0 column at s=sectors with u=1
- `BallTexture::generate` rasterizes onto a CPU bitmap then uploads once — no per-frame GPU cost
- Texture unit binding uses `setInt("uTexture", 0)` since `sampler2D` uniforms require an integer slot index, not a float

---

## Source File Index

| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point, game loop, GLFW callbacks |
| `src/Shader.hpp/cpp` | GLSL program wrapper + uniform setters |
| `src/Mesh.hpp/cpp` | VAO/VBO/EBO + UV sphere generator |
| `src/Camera.hpp/cpp` | Spherical-coordinate orbiting camera |
| `src/Texture.hpp/cpp` | 2D GL texture upload and bind |
| `src/BallTexture.hpp/cpp` | Runtime numbered ball texture via stb_truetype |
| `src/util.hpp` | `chdirToExe()` — portable asset path helper |
| `shaders/ball.vert` | Sphere vertex shader (MVP + UV passthrough) |
| `shaders/ball.frag` | Phong lighting + texture sampling |
| `shaders/triangle.vert/frag` | M1 leftover (used by triangle.vert/frag in main shaders/) |
| `unit_tests/m1_triangle.cpp` | M1 standalone regression test |
| `unit_tests/shaders/` | Shaders used exclusively by unit tests |
| `third_party/stb/stb_truetype.h` | Vendored font rasterizer |
| `assets/fonts/DejaVuSans-Bold.ttf` | Font used for ball number labels |
| `plan/plan_v0.md` | Full architecture doc and milestone definitions |
| `plan/system.txt` | Auto-generated system/library version report |
| `plan/system_detect.sh` | Script that regenerates system.txt |
