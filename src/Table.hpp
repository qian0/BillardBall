#pragma once

// Table dimensions and shared geometry constants used throughout the game.
struct Table
{
    static constexpr float kLength   = 9.00f; // playing surface length (X axis)
    static constexpr float kWidth    = 4.50f; // playing surface width (Z axis)
    static constexpr float kCushionH = 0.12f; // cushion height above felt surface
    static constexpr float kCushionT = 0.10f; // cushion depth inward from the edge
    static constexpr float kBallR    = 0.10f; // ball radius (~1/45 of table length, matches real scale)
};
