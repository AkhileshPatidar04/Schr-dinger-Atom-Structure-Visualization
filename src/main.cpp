#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "OrbitalCloud.h"

static Camera camera(30.0);
static bool dragging = false;
static double lastX = 0, lastY = 0;

// Quantum numbers / nuclear charge, editable at runtime.
static int n = 3, l = 2, m = 0;
static double Z = 1.0;
static bool regenerate = true;
static bool takeScreenshot = false;

static void updateWindowTitle(GLFWwindow* window)
{
    const char* subshell = "spdfgh";
    char label = (l >= 0 && l < 6) ? subshell[l] : '?';
    char title[160];
    std::snprintf(title, sizeof(title), "Schrodinger Atom Structure Visualization - %d%c (n=%d, l=%d, m=%d, Z=%.1f)",
                  n, label, n, l, m, Z);
    glfwSetWindowTitle(window, title);
}

static void printState()
{
    const char* subshell = "spdfgh";
    char label = (l >= 0 && l < 6) ? subshell[l] : '?';
    std::cout << "Orbital: n=" << n << " l=" << l << " (" << n << label
               << ") m=" << m << "  Z=" << Z << std::endl;
}

static void clampQuantumNumbers()
{
    if (n < 1) n = 1;
    if (l > n - 1) l = n - 1;
    if (l < 0) l = 0;
    if (m > l) m = l;
    if (m < -l) m = -l;
}

static void saveScreenshot(GLFWwindow* window, const char* customName = nullptr)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) return;

    std::vector<unsigned char> pixels(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    char filename[128];
    if (customName) {
        std::snprintf(filename, sizeof(filename), "%s", customName);
    } else {
        const char* subshell = "spdfgh";
        char label = (l >= 0 && l < 6) ? subshell[l] : '?';
        std::snprintf(filename, sizeof(filename), "orbital_%d%c_m%d.bmp", n, label, m);
    }

    std::ofstream out(filename, std::ios::binary);
    if (!out) return;

    uint32_t rowSize = (width * 3 + 3) & ~3;
    uint32_t dataSize = rowSize * height;
    uint32_t fileSize = 54 + dataSize;

    unsigned char header[54] = {
        'B', 'M',
        0, 0, 0, 0,
        0, 0, 0, 0,
        54, 0, 0, 0,
        40, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 0,
        24, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    std::memcpy(&header[2], &fileSize, 4);
    std::memcpy(&header[18], &width, 4);
    std::memcpy(&header[22], &height, 4);
    std::memcpy(&header[34], &dataSize, 4);

    out.write(reinterpret_cast<const char*>(header), 54);
    std::vector<unsigned char> padding(rowSize - width * 3, 0);
    for (int y = 0; y < height; ++y) {
        out.write(reinterpret_cast<const char*>(&pixels[y * width * 3]), width * 3);
        if (!padding.empty()) out.write(reinterpret_cast<const char*>(padding.data()), padding.size());
    }
    std::cout << "[Saved Screenshot] " << filename << std::endl;
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (key == GLFW_KEY_ESCAPE) { glfwSetWindowShouldClose(window, true); return; }

    if (key == GLFW_KEY_S)      { takeScreenshot = true; return; }
    if (key == GLFW_KEY_UP)     { n++; regenerate = true; }
    if (key == GLFW_KEY_DOWN)   { n--; regenerate = true; }
    if (key == GLFW_KEY_RIGHT)  { l++; regenerate = true; }
    if (key == GLFW_KEY_LEFT)   { l--; regenerate = true; }
    if (key == GLFW_KEY_PERIOD) { m++; regenerate = true; }
    if (key == GLFW_KEY_COMMA)  { m--; regenerate = true; }
    if (key == GLFW_KEY_EQUAL)  { Z += 1.0; regenerate = true; }
    if (key == GLFW_KEY_MINUS)  { Z = std::max(1.0, Z - 1.0); regenerate = true; }

    if (regenerate) {
        clampQuantumNumbers();
        printState();
        updateWindowTitle(window);
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        dragging = (action == GLFW_PRESS);
        glfwGetCursorPos(window, &lastX, &lastY);
    }
}

static void cursorPosCallback(GLFWwindow*, double x, double y)
{
    if (dragging) {
        camera.Rotate(x - lastX, lastY - y);
    }
    lastX = x;
    lastY = y;
}

static void scrollCallback(GLFWwindow*, double, double yoff)
{
    camera.Zoom(yoff);
}

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1280, 800, "Atomic Orbital Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to init GLEW" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.03f, 0.03f, 0.06f, 1.0f);

    Shader shader("shaders/point.vert", "shaders/point.frag");
    OrbitalCloud cloud;

    std::cout <<
        "Controls:\n"
        "  Up/Down    : change n (principal quantum number)\n"
        "  Left/Right : change l (0..n-1  -> s,p,d,f,...)\n"
        "  ,/.        : change m (-l..l)\n"
        "  -/=        : change Z (nuclear charge)\n"
        "  S          : save screenshot (BMP)\n"
        "  Mouse drag : rotate camera\n"
        "  Scroll     : zoom\n"
        "  Esc        : quit\n" << std::endl;

    clampQuantumNumbers();
    printState();
    updateWindowTitle(window);

    int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        if (regenerate) {
            cloud.Generate(n, l, m, Z, 50000);
            regenerate = false;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) { glfwPollEvents(); continue; }
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), (float)width / (float)height, 0.1f, 2000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shader.Use();
        shader.SetMat4("projection", projection);
        shader.SetMat4("view", view);
        shader.SetVec3("viewPos", camera.GetPosition());
        shader.SetFloat("basePointSize", 6.0f);

        cloud.Draw();

        // Save screenshot if requested, or automatically on initial frame to produce preview
        if (takeScreenshot) {
            saveScreenshot(window);
            takeScreenshot = false;
        } else if (frameCount == 2) {
            saveScreenshot(window, "orbital_preview.bmp");
        }
        if (frameCount < 10) ++frameCount;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
