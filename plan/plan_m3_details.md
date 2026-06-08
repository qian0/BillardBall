# M3 — Table & Static Scene: Implementation Plan

_Written: 2026-06-07_

---

## Goal

Render the full static billiard scene: felt table surface, 4 cushion walls, and 16 balls in
standard 8-ball rack formation with the cue ball at the head spot. Camera can orbit the full
table. No physics or input yet.

---

## Scene Geometry

### Coordinate system
- Table lies in the **XZ plane** (Y = 0 is the felt surface).
- Table center at the world origin.
- Length along **X** (head at −X, foot at +X).
- Balls sit on the surface: center Y = BALL\_R.

### Constants (`Table.hpp`)
| Name | Value | Notes |
|------|-------|-------|
| `kLength` | 9.0 | playing surface length (X) |
| `kWidth` | 4.5 | playing surface width (Z) |
| `kCushionH` | 0.30 | cushion height above felt |
| `kCushionT` | 0.25 | cushion depth (inward) |
| `kBallR` | 0.25 | ball radius |

### Ball positions
- **Foot spot** (rack apex): `x = +kLength / 4 = 2.25`
- **Head spot** (cue ball): `x = −kLength / 4 = −2.25`, `z = 0`
- Row spacing along X: `kBallR × sqrt(3) ≈ 0.433`
- Column spacing along Z: `2 × kBallR = 0.50`

Standard rack (15 object balls, rows 0–4):
```
Row 0 (apex):  1
Row 1:         2  3
Row 2:         4  8  5    ← 8-ball always centre
Row 3:         6  9  7 10
Row 4:        15 11 13 12 14
```
Z position within each row: `z = (−row + 2×col) × kBallR`

### Cushions
Two mesh types, each a `Mesh::box`:

| Mesh | halfW | halfH | halfD |
|------|-------|-------|-------|
| Long cushion (×2, along X) | kLength/2 | kCushionH/2 | kCushionT/2 |
| Short cushion (×2, along Z) | kCushionT/2 | kCushionH/2 | kWidth/2 |

Translation for each:
- North long: `(0,  kCushionH/2, −kWidth/2 + kCushionT/2)`
- South long: `(0,  kCushionH/2, +kWidth/2 − kCushionT/2)`
- West short: `(−kLength/2 + kCushionT/2, kCushionH/2, 0)`
- East short: `(+kLength/2 − kCushionT/2, kCushionH/2, 0)`

Corners overlap between long and short cushions — acceptable for M3.

---

## Code Changes

### New: `src/Table.hpp`
Plain header with the constants above as `static constexpr float`.

### Modified: `src/Mesh.hpp / Mesh.cpp`
Add two new factory methods:
- `Mesh::quad(halfW, halfH)` — flat XZ quad, normal (0,1,0)
- `Mesh::box(halfW, halfH, halfD)` — 6-face AABB, per-face normals, 24 verts / 36 indices

### Modified: `shaders/ball.frag`
Add `uniform bool uUseDecal`. Wrap the entire decal-projection block in `if (uUseDecal)`.
Without this, any surface whose world normal has `N.z > 0` (e.g. the south cushion inner face)
would incorrectly show the ball disc texture.

### Modified: `src/BallTexture.cpp`
Skip disc and number rendering when `number == 0` (cue ball). Produces a fully transparent
texture; `uBallColor` (white) provides the visible colour.

### Modified: `src/main.cpp`
Full M3 scene:
1. Camera at radius 10, phi 0.9, theta 0.3 (3/4 overhead view of the whole table).
2. Build meshes: 1 sphere (shared), 1 table quad, 1 long-cushion box, 1 short-cushion box.
3. Generate 16 `Texture` objects via `BallTexture::generate` (one per ball number 0–15).
4. Compute 16 ball positions as described above.
5. Render loop:
   - Set `uUseDecal = 0`; draw table (felt green) and 4 cushions (rail colour).
   - Set `uUseDecal = 1`; for each ball: bind its texture, set colour and model matrix, draw sphere.
   - `uNormalMatrix` is `mat3(1.0f)` for all objects (pure translation, no scale/rotation).

---

## Colours
| Object | RGB |
|--------|-----|
| Felt surface | (0.08, 0.38, 0.08) |
| Cushion | (0.05, 0.30, 0.05) |
| Rail (table outer quad, future) | — |
| Ball 0 cue | (1.00, 1.00, 1.00) |
| Ball 1 yellow | (0.95, 0.80, 0.00) |
| Ball 2 blue | (0.05, 0.15, 0.80) |
| Ball 3 red | (0.80, 0.05, 0.05) |
| Ball 4 purple | (0.40, 0.00, 0.55) |
| Ball 5 orange | (0.90, 0.40, 0.00) |
| Ball 6 green | (0.05, 0.45, 0.10) |
| Ball 7 maroon | (0.50, 0.05, 0.10) |
| Ball 8 black | (0.08, 0.08, 0.08) |
| Balls 9–15 | same RGB as 1–7 (stripes handled in M8) |

---

## Test Criteria (from plan_v0.md)
- All 16 balls visible in standard rack formation.
- Camera orbits the full table without clipping.
- Correct relative scale (balls look like billiard balls on a pool table).
- Cue ball distinct (plain white, no number).
- 8-ball in centre of rack row 2.

---

## Deferred to Later Milestones
- Pocket gaps in cushions (M7)
- Stripe visual distinction (M8)
- Rail wood-grain texture (M8)
- Outer table border / leg geometry (M8)
