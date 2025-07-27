#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <array>

// Forward declare WindowData struct
struct WindowData;

// Function declarations
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(WindowData &windowData, float deltaTime);

void generateSphere(std::vector<float> &vertices, float radius, int sectorCount, int stackCount);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Lighting
glm::vec3 lightPos(0.0f, 0.0f, 3.5f);

// Struct to hold window-specific data
struct WindowData {
    GLFWwindow *window;
    Camera camera;
    float lastX, lastY;
    bool firstMouse;
    Shader *lightingShader;
    Shader *lightCubeShader;
};

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create the windows first
    GLFWwindow *window1 = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL - Phong", NULL, NULL);
    GLFWwindow *window2 = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL - Gouraud", NULL, NULL);
    GLFWwindow *window3 = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL - Flat", NULL, NULL);

    // Check window creation
    if (window1 == NULL || window2 == NULL || window3 == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize WindowData array
    WindowData windows[3];

    // Setup each window data
    for (int i = 0; i < 3; i++) {

        glm::vec3 cameraPos = glm::vec3(0.5f, 0.5f, 2.0f);  // Position camera slightly to the right and above
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // Look at center
        windows[i].window = (i == 0) ? window1 : ((i == 1) ? window2 : window3);
        windows[i].camera = Camera(cameraPos);
        windows[i].camera.Front = glm::normalize(cameraTarget - cameraPos);
        windows[i].lastX = SCR_WIDTH / 2.0f;
        windows[i].lastY = SCR_HEIGHT / 2.0f;
        windows[i].firstMouse = true;
        windows[i].lightingShader = nullptr;
        windows[i].lightCubeShader = nullptr;
    }

    // Set callbacks for each window
    for (int i = 0; i < 3; i++) {
        glfwMakeContextCurrent(windows[i].window);
        glfwSetWindowUserPointer(windows[i].window, &windows[i]);

        glfwSetFramebufferSizeCallback(windows[i].window, framebuffer_size_callback);

        // Use lambdas to pass the proper window data to callbacks
        glfwSetCursorPosCallback(windows[i].window, [](GLFWwindow *w, double xpos, double ypos) {
            WindowData *data = static_cast<WindowData *>(glfwGetWindowUserPointer(w));

            if (data->firstMouse) {
                data->lastX = xpos;
                data->lastY = ypos;
                data->firstMouse = false;
            }

            float xoffset = xpos - data->lastX;
            float yoffset = data->lastY - ypos;

            data->lastX = xpos;
            data->lastY = ypos;

            data->camera.ProcessMouseMovement(xoffset, yoffset);
        });

        glfwSetScrollCallback(windows[i].window, [](GLFWwindow *w, double xoffset, double yoffset) {
            WindowData *data = static_cast<WindowData *>(glfwGetWindowUserPointer(w));
            data->camera.ProcessMouseScroll(static_cast<float>(yoffset));
        });

        // Load GLAD for each window
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }

        // Configure global OpenGL state
        glEnable(GL_DEPTH_TEST);
    }

    // Load different shaders for each window
    glfwMakeContextCurrent(windows[0].window);
    windows[2].lightingShader = new Shader("2.2.basic_lighting.vs", "2.2.basic_lighting.fs"); // Phong
    windows[2].lightCubeShader = new Shader("2.2.light_cube.vs", "2.2.light_cube.fs");

    glfwMakeContextCurrent(windows[1].window);
    windows[1].lightingShader = new Shader("2.2.gouraud_lighting.vs", "2.2.gouraud_lighting.fs"); // Gouraud
    windows[1].lightCubeShader = new Shader("2.2.light_cube.vs", "2.2.light_cube.fs");

    glfwMakeContextCurrent(windows[2].window);
    windows[0].lightingShader = new Shader("2.2.flat_lighting.vs", "2.2.flat_lighting.fs"); // Flat
    windows[0].lightCubeShader = new Shader("2.2.light_cube.vs", "2.2.light_cube.fs");

    // Generate sphere vertices once
    std::vector<float> sphereVertices;
    float radius = 0.5f;
    int sectors = 16; // Low poly
    int stacks = 8; // Low poly
    generateSphere(sphereVertices, radius, sectors, stacks);

    // Create cube vertices for light source
    float vertices[] = {
        // positions          // normals
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
    };

    // Create VAOs and VBOs for each window
    unsigned int sphereVAO[3], VBO[3];
    unsigned int lightCubeVAO[3], lightVBO[3];

    // Set up buffers for each window
    for (int i = 0; i < 3; i++) {
        glfwMakeContextCurrent(windows[i].window);

        // Configure sphere VAO and VBO
        glGenVertexArrays(1, &sphereVAO[i]);
        glGenBuffers(1, &VBO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

        glBindVertexArray(sphereVAO[i]);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Configure light VAO
        glGenVertexArrays(1, &lightCubeVAO[i]);
        glGenBuffers(1, &lightVBO[i]);

        glBindBuffer(GL_ARRAY_BUFFER, lightVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(lightCubeVAO[i]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);
    }
    // After creating the windows and before the main loop:
    // Position windows side by side horizontally
    int screenWidth = 0;
    int screenHeight = 0;

    // Get primary monitor resolution
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    screenWidth = mode->width;
    screenHeight = mode->height;

    // Set window positions
    // First window (left)
    glfwSetWindowPos(window1, 0, screenHeight/2 - SCR_HEIGHT/2);
    // Second window (center)
    glfwSetWindowPos(window2, screenWidth/2 - SCR_WIDTH/2 + 50, screenHeight/2 - SCR_HEIGHT/2);
    // Third window (right)
    glfwSetWindowPos(window3, screenWidth/2 + 250, screenHeight/2 - SCR_HEIGHT/2);


    // Render loop
    while (!glfwWindowShouldClose(windows[0].window) &&
           !glfwWindowShouldClose(windows[1].window) &&
           !glfwWindowShouldClose(windows[2].window)) {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Update light position
        float lightRadius = 2.0f;
        float lightSpeed = 1.0f;
        float angle = static_cast<float>(glfwGetTime()) * lightSpeed;
        glm::vec3 lightCenter = glm::vec3(0.0f, 0.0f, 3.5f);
        lightPos.x = lightCenter.x + sin(angle) * lightRadius;
        lightPos.y = lightCenter.y + cos(angle) * lightRadius;

        // Render each window
        for (int i = 0; i < 3; i++) {
            // Skip if window is closed
            if (glfwWindowShouldClose(windows[i].window))
                continue;

            glfwMakeContextCurrent(windows[i].window);

            // Process input
            processInput(windows[i], deltaTime);

            // Clear the screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Activate lighting shader and set uniforms
            windows[i].lightingShader->use();
            windows[i].lightingShader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
            windows[i].lightingShader->setVec3("lightColor", 1.2f, 1.2f, 1.2f);
            windows[i].lightingShader->setVec3("lightPos", lightPos);
            windows[i].lightingShader->setVec3("viewPos", windows[i].camera.Position);

            // View/projection transformations
            glm::mat4 projection = glm::perspective(glm::radians(windows[i].camera.Zoom),
                                                    (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = windows[i].camera.GetViewMatrix();
            windows[i].lightingShader->setMat4("projection", projection);
            windows[i].lightingShader->setMat4("view", view);

            // World transformation
            glm::mat4 model = glm::mat4(1.0f);
            windows[i].lightingShader->setMat4("model", model);

            // Render the sphere
            glBindVertexArray(sphereVAO[i]);
            glDrawArrays(GL_TRIANGLES, 0, sphereVertices.size() / 6);

            // Render the light cube
            windows[i].lightCubeShader->use();
            windows[i].lightCubeShader->setMat4("projection", projection);
            windows[i].lightCubeShader->setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
            windows[i].lightCubeShader->setMat4("model", model);

            glBindVertexArray(lightCubeVAO[i]);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Swap buffers
            glfwSwapBuffers(windows[i].window);
        }

        // Poll events
        glfwPollEvents();
    }

    // Clean up shaders
    for (int i = 0; i < 3; i++) {
        delete windows[i].lightingShader;
        delete windows[i].lightCubeShader;
    }

    // Clean up VAOs and VBOs
    for (int i = 0; i < 3; i++) {
        glDeleteVertexArrays(1, &sphereVAO[i]);
        glDeleteVertexArrays(1, &lightCubeVAO[i]);
        glDeleteBuffers(1, &VBO[i]);
        glDeleteBuffers(1, &lightVBO[i]);
    }

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// Process input for a specific window
void processInput(WindowData &windowData, float deltaTime) {
    if (glfwGetKey(windowData.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(windowData.window, true);

    if (glfwGetKey(windowData.window, GLFW_KEY_W) == GLFW_PRESS)
        windowData.camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(windowData.window, GLFW_KEY_S) == GLFW_PRESS)
        windowData.camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(windowData.window, GLFW_KEY_A) == GLFW_PRESS)
        windowData.camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(windowData.window, GLFW_KEY_D) == GLFW_PRESS)
        windowData.camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Callback for window resize
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Sphere generation function
void generateSphere(std::vector<float> &vertices, float radius, int sectorCount, int stackCount) {
#ifndef M_PI
            #define M_PI 3.14159265358979323846
#endif

    std::vector<float> tempVertices;
    float x, y, z, xy; // vertex position
    float nx, ny, nz; // vertex normal

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    // Generate vertices for the sphere (positions and normals)
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            // vertex position
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // normalized vertex normal
            nx = x / radius;
            ny = y / radius;
            nz = z / radius;

            // add vertex position and normal
            tempVertices.push_back(x);
            tempVertices.push_back(y);
            tempVertices.push_back(z);
            tempVertices.push_back(nx);
            tempVertices.push_back(ny);
            tempVertices.push_back(nz);
        }
    }

    // Generate triangles from the vertices
    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            // Get the indices of the quad corners
            int k1 = i * (sectorCount + 1) + j;
            int k2 = k1 + 1;
            int k3 = (i + 1) * (sectorCount + 1) + j;
            int k4 = k3 + 1;

            // Create two triangles for each quad
            if (i != 0) {
                // Skip for the top stack to avoid degenerate triangles
                for (int k = 0; k < 6; ++k) vertices.push_back(tempVertices[k1 * 6 + k]);
                for (int k = 0; k < 6; ++k) vertices.push_back(tempVertices[k2 * 6 + k]);
                for (int k = 0; k < 6; ++k) vertices.push_back(tempVertices[k3 * 6 + k]);
            }

            if (i != stackCount - 1) {
                // Skip for the bottom stack
                for (int k = 0; k < 6; ++k) vertices.push_back(tempVertices[k2 * 6 + k]);
                for (int k = 0; k < 6; ++k) vertices.push_back(tempVertices[k4 * 6 + k]);
                for (int k = 0; k < 6; ++k) vertices.push_back(tempVertices[k3 * 6 + k]);
            }
        }
    }
}
