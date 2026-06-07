#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "BallTexture.hpp"

#include <vector>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <cstring>

// Reads an entire binary file into a byte vector.
static std::vector<unsigned char> readBinary(const std::string &path)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        throw std::runtime_error("BallTexture: cannot open font: " + path);
    }
    auto size = f.tellg();
    f.seekg(0);
    std::vector<unsigned char> buf(size);
    f.read(reinterpret_cast<char *>(buf.data()), size);
    return buf;
}

// Fills a rectangular region of the RGBA bitmap with a solid colour.
static void fillRect(std::vector<unsigned char> &pixels, int texSize,
                     int x0, int y0, int x1, int y1,
                     unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            int idx = (y * texSize + x) * 4;
            pixels[idx + 0] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
            pixels[idx + 3] = a;
        }
    }
}

// Draws a filled circle into the RGBA bitmap, used for the white number disc.
static void fillCircle(std::vector<unsigned char> &pixels, int texSize,
                       int cx, int cy, int radius,
                       unsigned char r, unsigned char g, unsigned char b)
{
    for (int y = cy - radius; y <= cy + radius; ++y) {
        for (int x = cx - radius; x <= cx + radius; ++x) {
            if (x < 0 || x >= texSize || y < 0 || y >= texSize) { continue; }
            float dx = static_cast<float>(x - cx);
            float dy = static_cast<float>(y - cy);
            if (dx * dx + dy * dy <= static_cast<float>(radius * radius)) {
                int idx = (y * texSize + x) * 4;
                pixels[idx + 0] = r;
                pixels[idx + 1] = g;
                pixels[idx + 2] = b;
                pixels[idx + 3] = 255;
            }
        }
    }
}

// Blends a greyscale stb_truetype glyph bitmap onto the RGBA texture in black ink.
static void blitGlyph(std::vector<unsigned char> &pixels, int texSize,
                      const unsigned char *glyph, int gw, int gh, int ox, int oy)
{
    for (int gy = 0; gy < gh; ++gy) {
        for (int gx = 0; gx < gw; ++gx) {
            int px = ox + gx;
            int py = oy + gy;
            if (px < 0 || px >= texSize || py < 0 || py >= texSize) { continue; }
            unsigned char alpha = glyph[gy * gw + gx];
            if (alpha == 0) { continue; }
            int idx = (py * texSize + px) * 4;
            // Alpha-blend black glyph onto existing pixel
            float a = alpha / 255.0f;
            pixels[idx + 0] = static_cast<unsigned char>(pixels[idx + 0] * (1.0f - a));
            pixels[idx + 1] = static_cast<unsigned char>(pixels[idx + 1] * (1.0f - a));
            pixels[idx + 2] = static_cast<unsigned char>(pixels[idx + 2] * (1.0f - a));
            pixels[idx + 3] = 255;
        }
    }
}

// Generates a numbered billiard ball texture: coloured background, white disc, black number.
Texture BallTexture::generate(int number, glm::vec3 color, const std::string &fontPath, int size)
{
    std::vector<unsigned char> pixels(size * size * 4);

    // Transparent background — ball colour is supplied via uBallColor uniform
    fillRect(pixels, size, 0, 0, size, size, 0, 0, 0, 0);

    // White disc centred on the texture — the label circle
    int cx = size / 2;
    int cy = size / 2;
    int discRadius = size / 4;
    fillCircle(pixels, size, cx, cy, discRadius, 255, 255, 255);

    // Load and rasterize the number using stb_truetype
    auto fontData = readBinary(fontPath);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontData.data(), stbtt_GetFontOffsetForIndex(fontData.data(), 0))) {
        throw std::runtime_error("BallTexture: stbtt_InitFont failed for: " + fontPath);
    }

    std::string label = std::to_string(number);
    float fontSize = static_cast<float>(discRadius) * 1.1f;
    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);

    // Measure total advance width for horizontal centering
    int totalW = 0;
    for (char c : label) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
        totalW += static_cast<int>(advance * scale);
    }

    // Measure the tight pixel bounding box of the label for vertical centering.
    // Using the full line height (ascent+descent) would push digits upward because
    // they have no descenders — the tight box gives the actual rendered extent.
    int labelTop = INT_MAX, labelBot = INT_MIN;
    for (char c : label) {
        int ix0, iy0, ix1, iy1;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &ix0, &iy0, &ix1, &iy1);
        if (iy0 < labelTop) { labelTop = iy0; }
        if (iy1 > labelBot) { labelBot = iy1; }
    }
    // baseline such that the tight box midpoint lands on cy
    int baseline = cy - (labelTop + labelBot) / 2;
    int penX = cx - totalW / 2;

    // Rasterize each character of the label and blit it onto the disc
    for (char c : label) {
        int gw, gh, ox, oy;
        unsigned char *bitmap = stbtt_GetCodepointBitmap(
            &font, scale, scale, c, &gw, &gh, &ox, &oy);

        blitGlyph(pixels, size, bitmap, gw, gh, penX + ox, baseline + oy);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
        penX += static_cast<int>(advance * scale);

        stbtt_FreeBitmap(bitmap, nullptr);
    }

    return Texture::fromRGBA(pixels.data(), size, size);
}
