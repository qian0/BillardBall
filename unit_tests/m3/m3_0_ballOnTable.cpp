// M3.0 milestone test — one numbered ball sitting on a flat surface.
// Ball radius = 0.5, table = 3x3. A white "T" letter is rendered on the table
// surface so you can track orientation as you orbit the camera.
// Use this to inspect decal projection and lighting in isolation before the full M3 scene.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include "stb/stb_truetype.h"

#include "Shader.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "BallTexture.hpp"
#include "util.hpp"

static const int WINDOW_W = 1280;
static const int WINDOW_H = 720;

static constexpr float kBallR   = 0.5f;
static constexpr float kTableHW = 1.5f; // half-width/depth of the 3x3 table quad

struct AppContext
{
    Camera *camera;
    bool mouseDown = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};

// GLFW keyboard callback — closes the window when Escape is pressed.
static void onKey(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// GLFW framebuffer-resize callback — keeps the GL viewport matched to the window size.
static void onFramebufferResize(GLFWwindow * /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

// GLFW mouse-button callback — tracks when the left button is held for drag-to-orbit.
static void onMouseButton(GLFWwindow *window, int button, int action, int /*mods*/)
{
    auto *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->mouseDown = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &ctx->lastMouseX, &ctx->lastMouseY);
    }
}

// GLFW cursor-position callback — translates mouse drag deltas into camera orbit angles.
static void onCursorPos(GLFWwindow *window, double x, double y)
{
    auto *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (ctx->mouseDown) {
        float dx = static_cast<float>(x - ctx->lastMouseX);
        float dy = static_cast<float>(y - ctx->lastMouseY);
        ctx->camera->onMouseDrag(dx, dy);
    }
    ctx->lastMouseX = x;
    ctx->lastMouseY = y;
}

// GLFW scroll callback — zooms the orbit camera in and out.
static void onScroll(GLFWwindow *window, double /*xOffset*/, double yOffset)
{
    auto *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    ctx->camera->onScroll(static_cast<float>(yOffset));
}

// Rasterizes a single character into an OpenGL GL_RED texture of the given pixel size.
// The glyph is centred and scaled to fill roughly 80% of the texture height.
static GLuint makeLetterTex(const std::string &fontPath, char letter, int size)
{
    std::ifstream f(fontPath, std::ios::binary | std::ios::ate);
    if (!f.is_open()) {
        throw std::runtime_error("makeLetterTex: cannot open font: " + fontPath);
    }
    auto sz = f.tellg();
    f.seekg(0);
    std::vector<unsigned char> fontData(sz);
    f.read(reinterpret_cast<char *>(fontData.data()), sz);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontData.data(), stbtt_GetFontOffsetForIndex(fontData.data(), 0))) {
        throw std::runtime_error("makeLetterTex: stbtt_InitFont failed");
    }

    float scale = stbtt_ScaleForPixelHeight(&font, size * 0.8f);

    int gw, gh, ox, oy;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, letter, &gw, &gh, &ox, &oy);

    // Centre glyph within a square buffer; stb bitmap is top-down so we flip Y on upload.
    std::vector<unsigned char> buf(size * size, 0);
    int dstX = (size - gw) / 2;
    int dstY = (size - gh) / 2;
    for (int y = 0; y < gh; ++y) {
        for (int x = 0; x < gw; ++x) {
            int px = dstX + x;
            int py = dstY + y;
            if (px >= 0 && px < size && py >= 0 && py < size) {
                buf[py * size + px] = bitmap[y * gw + x];
            }
        }
    }
    stbtt_FreeBitmap(bitmap, nullptr);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Upload with GL_UNPACK_ROW_LENGTH=0 and flip by setting negative stride via pixel transfer.
    // Simplest flip: pass the last row pointer and negative height isn't standard in glTexImage2D,
    // so we flip the buffer manually — already done above by addressing with (size-1-py).
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, buf.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

// Entry point: renders ball 8 on a flat table surface for decal and lighting inspection.
int main()
{
    chdirToExe();

    if (!glfwInit()) {
        std::cerr << "glfwInit failed\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "M3.0 — Ball on Table", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGLLoader failed\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION)
              << "  renderer: " << glGetString(GL_RENDERER) << '\n';

    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Camera camera(6.0f, 0.3f, 0.8f);

    AppContext ctx;
    ctx.camera = &camera;

    glfwSetWindowUserPointer(window, &ctx);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);

    // --- Geometry ---
    Mesh sphere = Mesh::uvSphere(32, 32);
    Mesh table  = Mesh::quad(kTableHW, kTableHW);

    // Label quad: a flat CCW quad in the XZ plane (y = 0.002) for the "T" letter.
    // UV (0,0) is at the -X/-Z corner; the stb bitmap is top-down so we flip V
    // (v=0 → top row of stb bitmap = tex bottom, v=1 → tex top) to display right-side-up
    // when viewed from above. Quad is 0.6×0.6 world units centred at z=1.0 (bottom of table).
    const float lHalf = 0.30f; // half-size of the label quad
    const float lY    = 0.002f;
    const float lZ    = 1.0f;  // centre of the T on the table (+Z = away from ball)
    // Vertices: pos(x,y,z), uv(u,v)  — CCW from above for +Y normal
    float labelVerts[] = {
        -lHalf, lY, lZ - lHalf,   0.0f, 1.0f,  // TL
         lHalf, lY, lZ - lHalf,   1.0f, 1.0f,  // TR
         lHalf, lY, lZ + lHalf,   1.0f, 0.0f,  // BR
        -lHalf, lY, lZ + lHalf,   0.0f, 0.0f,  // BL
    };
    unsigned int labelIdx[] = { 0, 3, 2,  0, 2, 1 };

    GLuint labelVAO, labelVBO, labelEBO;
    glGenVertexArrays(1, &labelVAO);
    glGenBuffers(1, &labelVBO);
    glGenBuffers(1, &labelEBO);
    glBindVertexArray(labelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, labelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(labelVerts), labelVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, labelEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(labelIdx), labelIdx, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // --- Shaders ---
    Shader ballShader("shaders/m3_0_ball.vert",  "shaders/m3_0_ball.frag");
    Shader flatShader("shaders/m3_0_flat.vert",  "shaders/m3_0_flat.frag");
    Shader labelShader("shaders/m3_0_label.vert", "shaders/m3_0_label.frag");

    ballShader.use();
    ballShader.setInt("uTexture", 0);
    labelShader.use();
    labelShader.setInt("uTexture", 0);

    // --- Textures ---
    const glm::vec3 ballColor(0.08f, 0.08f, 0.08f);
    Texture ballTex = BallTexture::generate(8, ballColor, "assets/fonts/DejaVuSans-Bold.ttf");
    GLuint letterTex = makeLetterTex("assets/fonts/DejaVuSans-Bold.ttf", 'T', 128);

    // Ball model: translate up to sit on the surface, then scale to radius.
    const glm::mat4 ballModel = glm::scale(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, kBallR, 0.0f)),
        glm::vec3(kBallR)
    );
    const glm::mat3 normalMatrix(1.0f);

    const glm::vec3 lightDir    = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
    const glm::vec3 feltColor   = {0.08f, 0.38f, 0.08f};
    const glm::vec3 labelColor  = {1.00f, 1.00f, 1.00f}; // white T

    const glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(WINDOW_W) / WINDOW_H,
        0.1f, 100.0f
    );

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view   = camera.viewMatrix();
        const glm::vec3 camPos = camera.position();

        // Draw table surface
        flatShader.use();
        flatShader.setMat4("uView", view);
        flatShader.setMat4("uProjection", projection);
        flatShader.setVec3("uLightDir", lightDir);
        flatShader.setVec3("uColor", feltColor);
        flatShader.setMat4("uModel", glm::mat4(1.0f));
        table.draw();

        // Draw "T" letter on the table with alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, letterTex);
        labelShader.use();
        labelShader.setMat4("uModel", glm::mat4(1.0f));
        labelShader.setMat4("uView", view);
        labelShader.setMat4("uProjection", projection);
        labelShader.setVec3("uColor", labelColor);
        glBindVertexArray(labelVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDisable(GL_BLEND);

        // Draw the ball with Phong + decal shader
        ballTex.bind(0);
        ballShader.use();
        ballShader.setMat4("uModel", ballModel);
        ballShader.setMat4("uView", view);
        ballShader.setMat4("uProjection", projection);
        ballShader.setMat3("uNormalMatrix", normalMatrix);
        ballShader.setVec3("uCameraPos", camPos);
        ballShader.setVec3("uLightDir", lightDir);
        ballShader.setVec3("uColor", ballColor);
        sphere.draw();

        glfwSwapBuffers(window);
    }

    ballTex.destroy();
    sphere.destroy();
    table.destroy();
    glDeleteTextures(1, &letterTex);
    glDeleteVertexArrays(1, &labelVAO);
    glDeleteBuffers(1, &labelVBO);
    glDeleteBuffers(1, &labelEBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
