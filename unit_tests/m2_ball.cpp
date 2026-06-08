// M2 milestone test — renders a single numbered billiard ball (8-ball) with Phong shading
// and a decal-projected number disc to verify the sphere mesh, camera, and ball texture pipeline.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

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

    GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "M2 — Numbered Ball", nullptr, nullptr);
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
    Shader shader("shaders/m2_ball.vert", "shaders/m2_ball.frag");

    Texture ballTex = BallTexture::generate(8, glm::vec3(0.08f, 0.08f, 0.08f),
                                            "assets/fonts/DejaVuSans-Bold.ttf");

    const glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
    const glm::vec3 ballColor(0.08f, 0.08f, 0.08f);

    const glm::mat4 model(1.0f);
    const glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
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

        glfwSwapBuffers(window);
    }

    ballTex.destroy();
    sphere.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
