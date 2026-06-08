// M3.1 — exact clone of the M2 ball test, living inside the m3 subfolder.
// Purpose: verify that the m3 build environment reproduces M2's correct
// decal behaviour before we modify anything.  No table, no second shader.

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

struct AppContext
{
    Camera *camera;
    bool mouseDown = false;
    bool rightMouseDown = false;
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

// GLFW mouse-button callback — tracks left button for orbit, right button for pan.
static void onMouseButton(GLFWwindow *window, int button, int action, int /*mods*/)
{
    auto *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->mouseDown = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &ctx->lastMouseX, &ctx->lastMouseY);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        ctx->rightMouseDown = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &ctx->lastMouseX, &ctx->lastMouseY);
    }
}

// GLFW cursor-position callback — left drag orbits, right drag pans.
static void onCursorPos(GLFWwindow *window, double x, double y)
{
    auto *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    float dx = static_cast<float>(x - ctx->lastMouseX);
    float dy = static_cast<float>(y - ctx->lastMouseY);
    if (ctx->mouseDown) {
        ctx->camera->onMouseDrag(dx, dy);
    }
    if (ctx->rightMouseDown) {
        ctx->camera->onMousePan(dx, dy);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size, size, 0, GL_RED, GL_UNSIGNED_BYTE, buf.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

// Entry point: renders a single lit, numbered billiard ball until Escape is pressed.
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

    GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "M3.1 — Ball clone of M2", nullptr, nullptr);
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

    Camera camera(5.0f, 0.0f, 0.4f);

    AppContext ctx;
    ctx.camera = &camera;

    glfwSetWindowUserPointer(window, &ctx);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);

    Mesh sphere = Mesh::uvSphere(32, 32);
    Mesh table  = Mesh::quad(2.0f, 2.0f);
    Shader shader("shaders/m3_1_ball.vert", "shaders/m3_1_ball.frag");
    Shader flatShader("shaders/m3_0_flat.vert", "shaders/m3_0_flat.frag");
    Shader labelShader("shaders/m3_0_label.vert", "shaders/m3_0_label.frag");

    Texture ballTex = BallTexture::generate(8, glm::vec3(0.08f, 0.08f, 0.08f),
                                            "assets/fonts/DejaVuSans-Bold.ttf");
    GLuint letterTex = makeLetterTex("assets/fonts/DejaVuSans-Bold.ttf", 'T', 128);

    // Label quad: flat CCW quad in the XZ plane sitting just above the table surface (y = -1 + epsilon).
    const float lHalf = 0.30f;
    const float lY    = 0.002f;
    const float lZ    =  1.0f;
    float labelVerts[] = {
        -lHalf, lY, lZ - lHalf,   0.0f, 1.0f,
         lHalf, lY, lZ - lHalf,   1.0f, 1.0f,
         lHalf, lY, lZ + lHalf,   1.0f, 0.0f,
        -lHalf, lY, lZ + lHalf,   0.0f, 0.0f,
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

    labelShader.use();
    labelShader.setInt("uTexture", 0);

    const glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
    const glm::vec3 ballColor(0.08f, 0.08f, 0.08f);

    constexpr float kBallR = 0.5f;
    const glm::mat4 model = glm::scale(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, kBallR, 0.0f)),
        glm::vec3(kBallR)
    );
    const glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    const glm::mat4 tableModel = glm::mat4(1.0f);
    const glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(WINDOW_W) / WINDOW_H,
        0.1f, 100.0f
    );

    shader.use();
    shader.setInt("uTexture", 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.12f, 0.18f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ballTex.bind(0);

        shader.use();
        shader.setMat4("uModel", model);
        shader.setMat4("uView", camera.viewMatrix());
        shader.setMat4("uProjection", projection);
        shader.setMat3("uNormalMatrix", normalMatrix);
        shader.setVec3("uCameraPos", camera.position());
        shader.setVec3("uLightDir", lightDir);
        shader.setVec3("uBallColor", ballColor);

        sphere.draw();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, letterTex);
        labelShader.use();
        labelShader.setMat4("uModel", glm::mat4(1.0f));
        labelShader.setMat4("uView", camera.viewMatrix());
        labelShader.setMat4("uProjection", projection);
        labelShader.setVec3("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
        glBindVertexArray(labelVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDisable(GL_BLEND);

        flatShader.use();
        flatShader.setMat4("uModel", tableModel);
        flatShader.setMat4("uView", camera.viewMatrix());
        flatShader.setMat4("uProjection", projection);
        flatShader.setVec3("uLightDir", lightDir);
        flatShader.setVec3("uColor", glm::vec3(0.08f, 0.38f, 0.08f));
        table.draw();

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
