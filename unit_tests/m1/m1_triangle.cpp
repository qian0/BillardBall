// M1 milestone test — opens a window and renders a hard-coded triangle to verify
// that the GLFW context, GLAD loader, VAO/VBO setup, and shader pipeline all work end-to-end.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Shader.hpp"
#include "util.hpp"

static const int WINDOW_W = 1280;
static const int WINDOW_H = 720;

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

// Entry point: sets up a window, uploads a triangle to the GPU, and renders it until Escape.
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

    GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "M1 — Triangle", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, onKey);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "gladLoadGLLoader failed\n";
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL " << glGetString(GL_VERSION)
              << "  renderer: " << glGetString(GL_RENDERER) << '\n';

    glViewport(0, 0, WINDOW_W, WINDOW_H);

    // Hard-coded triangle in NDC — no projection needed for a basic pipeline test
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f,
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    Shader shader("shaders/m1_triangle.vert", "shaders/m1_triangle.frag");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.12f, 0.18f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
