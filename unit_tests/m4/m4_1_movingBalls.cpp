// M4.1 — Single 8-ball bouncing off cushions.
// Tests cushion reflection and linear damping in isolation before adding sphere-sphere collisions.
// Press Space to fire the ball at a diagonal angle; it bounces and gradually stops.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <iostream>

#include "Shader.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "Texture.hpp"
#include "BallTexture.hpp"
#include "Physics.hpp"
#include "Table.hpp"
#include "util.hpp"
#include <glm/gtc/quaternion.hpp>

static const int WINDOW_W = 1280;
static const int WINDOW_H = 720;

struct AppContext
{
    Camera *camera;
    Physics *physics;
    bool mouseDown = false;
    bool rightMouseDown = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
};

// GLFW keyboard callback — Escape quits; Space fires the cue ball diagonally.
static void onKey(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    auto *ctx = static_cast<AppContext *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        ctx->physics->balls[0].vel = glm::vec3(3.5f, 0.0f, 2.0f);
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

// Entry point: one 8-ball on a table — press Space to fire, watch it bounce and stop.
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

    GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "M4.1 — Single Ball [Space to fire]", nullptr, nullptr);
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
    std::cout << "Press Space to fire the 8-ball diagonally.\n";

    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Camera camera(10.0f, 0.3f, 0.9f);

    // --- Physics — one active ball, rest pocketed so they are skipped ---
    Physics physics;
    physics.balls[0] = {
        .pos      = glm::vec3(0.0f, Table::kBallR, 0.0f),
        .vel      = glm::vec3(0.0f),
        .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        .radius   = Table::kBallR,
        .invMass  = 1.0f,
        .pocketed = false,
    };
    for (int i = 1; i < 16; ++i) {
        physics.balls[i] = {
            .pos      = glm::vec3(0.0f),
            .vel      = glm::vec3(0.0f),
            .rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            .radius   = Table::kBallR,
            .invMass  = 0.0f,
            .pocketed = true,
        };
    }

    AppContext ctx;
    ctx.camera  = &camera;
    ctx.physics = &physics;

    glfwSetWindowUserPointer(window, &ctx);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);

    // --- Geometry ---
    Mesh sphere = Mesh::uvSphere(32, 32);
    Mesh table  = Mesh::quad(Table::kLength / 2.0f, Table::kWidth / 2.0f);

    // --- Shaders ---
    Shader phong("shaders/phong.vert", "shaders/phong.frag");
    Shader flat("shaders/flat.vert",   "shaders/flat.frag");

    phong.use();
    phong.setInt("uTexture", 0);

    // --- Texture ---
    const glm::vec3 kBallColor(0.08f, 0.08f, 0.08f);
    Texture ballTex = BallTexture::generate(8, kBallColor, "assets/fonts/DejaVuSans-Bold.ttf");

    const glm::vec3 kLightDir  = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
    const glm::vec3 kFeltColor = glm::vec3(0.08f, 0.38f, 0.08f);

    const glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(WINDOW_W) / WINDOW_H,
        0.1f, 100.0f
    );

    double prevTime    = glfwGetTime();
    double accumulator = 0.0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Fixed-timestep accumulator — capped to avoid spiral-of-death on pauses
        double now = glfwGetTime();
        double dt  = std::min(now - prevTime, 0.25);
        prevTime   = now;
        accumulator += dt;
        while (accumulator >= Physics::kDt) {
            physics.step();
            accumulator -= Physics::kDt;
        }

        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view   = camera.viewMatrix();
        const glm::vec3 camPos = camera.position();

        // Draw table surface
        flat.use();
        flat.setMat4("uModel",      glm::mat4(1.0f));
        flat.setMat4("uView",       view);
        flat.setMat4("uProjection", projection);
        flat.setVec3("uLightDir",   kLightDir);
        flat.setVec3("uColor",      kFeltColor);
        table.draw();

        // Draw 8-ball at physics position with accumulated rolling rotation
        const BallState &b = physics.balls[0]; 
        glm::mat4 model = glm::translate(glm::mat4(1.0f), b.pos)
                        * glm::mat4_cast(b.rotation)
                        * glm::scale(glm::mat4(1.0f), glm::vec3(Table::kBallR));

        ballTex.bind(0);
        phong.use();
        phong.setMat4("uModel",        model);
        phong.setMat4("uView",         view);
        phong.setMat4("uProjection",   projection);
        phong.setMat3("uNormalMatrix", glm::mat3(glm::transpose(glm::inverse(model))));
        phong.setVec3("uCameraPos",    camPos);
        phong.setVec3("uLightDir",     kLightDir);  
        phong.setVec3("uColor",        kBallColor);
        sphere.draw();

        glfwSwapBuffers(window);
    }

    ballTex.destroy();
    sphere.destroy();
    table.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
