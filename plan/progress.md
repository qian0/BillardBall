# BillardBall — Progress

_Last updated: 2026-06-08_

---

## Milestones

| # | Name | Status |
|---|------|--------|
| M1 | Window + Triangle | Done |
| M2 | Sphere Renderer + Numbered Ball Texture | Done |
| M3 | Table & Static Scene | Done |
| M4 | Basic Physics (linear only) | Not started |
| M5 | Cue Stick Input | Not started |
| M6 | Rolling Friction & Spin | Not started |
| M7 | Basic Game Rules | Not started |
| M8 | Visual Polish | Not started |

---

## M1 — Window + Triangle (Done)

**What was built:**
- CMakeLists.txt: GLAD (vendored), GLFW (system), GLM (FetchContent 1.0.1)
- GLFW window with OpenGL 3.3 core context, vsync, Esc-to-quit
- Minimal vertex/fragment shader pair; hard-coded triangle
- `Shader` class: loads GLSL from disk, compiles, links, typed uniform setters
- `util.hpp`: `chdirToExe()` — changes working directory to binary location
- `unit_tests/m1/m1_triangle.cpp`: standalone regression test

---

## M2 — Sphere Renderer + Numbered Ball Texture (Done)

**What was built:**
- `Mesh`: VAO/VBO/EBO wrapper; `uvSphere(rings, sectors)` — position (loc 0), normal (loc 1), UV (loc 2)
- `Camera`: spherical-coordinate orbiting camera; left drag orbits (theta/phi), right drag pans (translates target), scroll zoom
- Phong shading: ambient + diffuse + specular, single directional light
- `Texture`: uploads RGBA bitmap to GPU, generates mipmaps
- `BallTexture`: runtime numbered ball texture via `stb_truetype`; disc + centred glyph
- `unit_tests/m2/m2_ball.cpp`: standalone regression test with frozen shaders

**Key decisions:**
- Switched from equirectangular UV to **decal projection** for the number disc — equirectangular distorts a circle into an oval.
- Number centering uses `stbtt_GetCodepointBitmapBox` (tight bounding box) rather than full line height — digits have no descenders so line-height centering pushed numbers up.
- Ball texture background is transparent (alpha=0); ball colour is supplied via `uBallColor` uniform.
- Cue ball (number=0) skips disc and glyph rendering entirely.

---

## M3 — Table & Static Scene (Done)

**What was built:**
- `Table.hpp`: scene constants — `kLength`, `kWidth`, `kCushionH`, `kCushionT`, `kBallR`
- `Mesh::quad(halfW, halfH)`: flat horizontal quad, position-only VBO, +Y normal (CCW from above)
- `Mesh::box(halfW, halfH, halfD)`: closed box, position-only VBO, face normals derived per-fragment via `dFdx`/`dFdy` in the flat shader — no duplicated vertices
- Two shader programs:
  - `phong.vert/frag` — balls: interpolated normals, Phong lighting, camera-space decal projection
  - `flat.vert/frag` — table/cushions: position-only VBO, derivative face normals, no specular
- `BallScene`: owns sphere mesh, 16 textures, rack positions; `create()` / `draw()` / `destroy()`
- `TableScene`: owns table quad and two cushion meshes (long × 2, short × 2 reused); `create()` / `draw()` / `destroy()`
- `main.cpp` refactored: GLFW boilerplate + `BallScene::create()` + `TableScene::create()` + render loop
- `unit_tests/m3/m3_1_ballOnTable.cpp`: isolated test — single ball on flat surface with a "T" orientation marker

**Key decisions:**
- Separate flat shader for table/cushions avoids storing normals in the VBO and eliminates accidental decal sampling on non-ball geometry.
- Box mesh uses 8 unique corner vertices; `dFdx`/`dFdy` computes per-triangle face normals in the fragment shader.
- `GL_CULL_FACE` enabled — all geometry is solid, viewed only from outside.
- Ball number decal projects from **world space** (`N.xz + 0.5`, `facing = N.y`), so the disc is painted onto the ball and stays fixed as the camera orbits.
- `discRadius` increased from `size/4` to `size/2` for better readability.
- Unit tests reorganised into `unit_tests/m1/`, `unit_tests/m2/`, `unit_tests/m3/` subfolders; each has its own `shaders/` and outputs to `build/unit_tests/<mX>/`.

**Bugs fixed post-M3:**
- `Mesh::uvSphere` indices were wound CW from the outside; swapped to CCW so `GL_CULL_FACE` culls the correct (interior) faces.
- `phong.frag` decal was projected in camera/view space — decal rotated with camera at 2× speed. Fixed to world-space projection.
- `BallScene::draw` used a shared identity `uNormalMatrix`; now computed per-ball as `transpose(inverse(mat3(model)))`.

---

## Source File Index

| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point, GLFW loop, camera callbacks |
| `src/BallScene.hpp/cpp` | 16 balls — mesh, textures, rack layout, draw |
| `src/TableScene.hpp/cpp` | Table surface + four cushions, draw |
| `src/Mesh.hpp/cpp` | VAO/VBO/EBO; `uvSphere`, `quad`, `box` |
| `src/Camera.hpp/cpp` | Spherical-coordinate orbiting camera |
| `src/Shader.hpp/cpp` | GLSL program wrapper + uniform setters |
| `src/Texture.hpp/cpp` | 2D GL texture upload and bind |
| `src/BallTexture.hpp/cpp` | Runtime numbered ball texture via stb_truetype |
| `src/Table.hpp` | Table dimension constants |
| `src/util.hpp` | `chdirToExe()` — portable asset path helper |
| `shaders/phong.vert/frag` | Phong lighting + world-space decal (balls) |
| `shaders/flat.vert/frag` | Derivative-normal flat shading (table, cushions) |
| `unit_tests/m1/` | M1 triangle regression test + shaders |
| `unit_tests/m2/` | M2 single numbered ball regression test + shaders |
| `unit_tests/m3/` | M3 isolated scene tests + shaders |
| `third_party/stb/stb_truetype.h` | Vendored font rasterizer |
| `assets/fonts/DejaVuSans-Bold.ttf` | Font for ball number labels |
| `plan/plan_v0.md` | Architecture doc and milestone definitions |
| `plan/plan_m3_details.md` | M3 detailed implementation plan |
