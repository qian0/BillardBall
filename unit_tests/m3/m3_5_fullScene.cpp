// M3.5 — Archive snapshot of the complete M3 scene (main.cpp as of milestone 3 completion).
// Full table with four cushions and all 16 balls in rack formation.
// Two shaders: phong (balls) and flat (table/cushions).
// Left drag orbits, right drag pans, scroll zooms.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "Shader.hpp"
#include "Camera.hpp"
#include "BallScene.hpp"
#include "TableScene.hpp"
#include "util.hpp"

static const int WINDOW_W = 1280;
static const int WINDOW_H = 720;

// Bundles mutable input state that GLFW callbacks need to share with the main loop.
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

// Entry point: builds the full static M3 scene (table, cushions, 16 balls) and runs the render loop.
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

    GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "M3.5 — Full Static Scene", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGLLoader failed\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION)
              << "  renderer: " << glGetString(GL_RENDERER) << '\n';

    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // all geometry is solid and viewed from outside

    // Camera starting position: 3/4 overhead view showing the full table
    Camera camera(10.0f, 0.3f, 0.9f);

    AppContext ctx;
    ctx.camera = &camera;

    glfwSetWindowUserPointer(window, &ctx);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetScrollCallback(window, onScroll);

    // --- Scene ---
    BallScene ballScene   = BallScene::create("assets/fonts/DejaVuSans-Bold.ttf");
    TableScene tableScene = TableScene::create();

    // --- Shaders ---
    Shader phong("shaders/phong.vert", "shaders/phong.frag");
    Shader flat("shaders/flat.vert",   "shaders/flat.frag");

    phong.use();
    phong.setInt("uTexture", 0);

    const glm::vec3 kLightDir = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));

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

        tableScene.draw(flat,  view, projection, kLightDir);
        ballScene.draw(phong, view, projection, kLightDir, camPos);

        glfwSwapBuffers(window);
    }

    ballScene.destroy();
    tableScene.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
