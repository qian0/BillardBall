# BillardBall

A 3D billiard ball game built with OpenGL 3.3 + GLFW + GLM on Linux.
Currently at **M3 (Table & Static Scene)** — 16 balls in rack formation on a felt table with cushions, orbiting camera, Phong-shaded balls with numbered decals.

## Dependencies

```bash
sudo apt install libglfw3-dev libgl-dev
```

GLM is fetched automatically by CMake at configure time.

## Build

```bash
# Game only (default)
cmake -S . -B build
cmake --build build -j$(nproc)

# Game + unit tests
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build -j$(nproc)

# Single target
cmake --build build --target BillardBall
cmake --build build --target m1_triangle
cmake --build build --target m2_ball
cmake --build build --target m3_1_ballOnTable
```

## Run

```bash
# Game
./build/BillardBall

# Unit tests (each milestone has its own subfolder)
./build/unit_tests/m1/m1_triangle
./build/unit_tests/m2/m2_ball
./build/unit_tests/m3/m3_1_ballOnTable
```

## Controls

| Input | Action |
|-------|--------|
| Esc | Quit |
| Left mouse drag | Orbit camera |
| Right mouse drag | Pan camera |
| Scroll wheel | Zoom in / out |

## Project Layout

```
BillardBall/
├── src/
│   ├── main.cpp              # entry point, GLFW loop
│   ├── BallScene.hpp/cpp     # 16 balls: mesh, textures, rack positions, draw
│   ├── TableScene.hpp/cpp    # table surface + four cushions, draw
│   ├── Mesh.hpp/cpp          # VAO/VBO/EBO; uvSphere, quad, box generators
│   ├── Camera.hpp/cpp        # spherical-coordinate orbiting camera
│   ├── Shader.hpp/cpp        # GLSL program wrapper + uniform setters
│   ├── Texture.hpp/cpp       # 2D GL texture upload and bind
│   ├── BallTexture.hpp/cpp   # runtime numbered ball texture via stb_truetype
│   ├── Table.hpp             # table dimension constants (kLength, kWidth, kBallR …)
│   └── util.hpp              # chdirToExe() — portable asset path helper
├── shaders/
│   ├── phong.vert/frag       # Phong lighting + camera-space decal projection (balls)
│   └── flat.vert/frag        # derivative-normal flat shading (table, cushions)
├── unit_tests/
│   ├── m1/                   # M1 triangle regression test + shaders
│   ├── m2/                   # M2 single numbered ball regression test + shaders
│   └── m3/                   # M3 isolated tests + shaders
│       └── m3_1_ballOnTable  # one ball on a surface with orientation marker
├── assets/fonts/             # DejaVuSans-Bold.ttf (ball number labels)
├── third_party/stb/          # stb_truetype.h (vendored)
└── plan/                     # architecture doc, milestone plan, progress log
```
