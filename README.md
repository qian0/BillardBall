# BillardBall

A simple 3D billiard ball game built with OpenGL 3.3 + GLFW + GLM on Linux.

## Dependencies

Install system packages before building:

```bash
sudo apt install libglfw3-dev libgl-dev
```

GLM is fetched automatically by CMake at configure time.

## Build

```bash
# Configure — game only (default)
cmake -S . -B build

# Configure — game + unit tests
cmake -S . -B build -DBUILD_TESTS=ON

# Compile everything
cmake --build build -- -j$(nproc)

# Or compile a specific target
cmake --build build --target BillardBall
cmake --build build --target m1_triangle
cmake --build build --target m2_ball
```

## Run

```bash
# Game
./build/BillardBall

# Unit tests
./build/unit_tests/m1_triangle
```

## Controls

| Input | Action |
|-------|--------|
| Esc | Quit |
| Left mouse drag | Orbit camera |
| Scroll wheel | Zoom in / out |
