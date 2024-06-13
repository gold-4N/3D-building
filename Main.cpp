#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb/stb_image.h"  // Include the header without defining STB_IMAGE_IMPLEMENTATION here
#include "shaderClass.h"
#include <iostream>
#include <vector>

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)";
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture1;
uniform bool lightOn;

void main()
{
    vec4 texColor = texture(texture1, TexCoord);
    if (lightOn) {
        FragColor = texColor * vec4(ourColor, 1.0);
    } else {
        FragColor = vec4(ourColor, 1.0);
    }
}
)";
GLuint createShaderProgram() {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Clean up shaders as they're linked into our program now and no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLFWwindow* initGLFWandGLAD() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set GLFW to use OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL 3D Surface with Buildings", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set the viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    return window;
}
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(int direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == GLFW_KEY_W)
            Position += Front * velocity;
        if (direction == GLFW_KEY_S)
            Position -= Front * velocity;
        if (direction == GLFW_KEY_A)
            Position -= Right * velocity;
        if (direction == GLFW_KEY_D)
            Position += Right * velocity;
        if (direction == GLFW_KEY_Q)
            Position += Up * velocity;
        if (direction == GLFW_KEY_E)
            Position -= Up * velocity;
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

int main() {
    // Initialize GLFW and GLAD
    GLFWwindow* window = initGLFWandGLAD();

    // Define the vertices and colors of the surface and buildings
    GLfloat vertices[] = {
        // positions          // colors           // texture coords
        // Surface
         // Surface (without texture coordinates)
    -5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
     5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,

    // Building 1 (with texture coordinates)
    -1.0f, 0.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
    -1.0f, 2.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     1.0f, 2.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
     1.0f, 0.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
    -1.0f, 0.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
    -1.0f, 2.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
     1.0f, 2.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
     1.0f, 0.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,

     // Building 2
      2.0f, 0.0f,  2.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
      2.0f, 3.0f,  2.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
      4.0f, 3.0f,  2.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
      4.0f, 0.0f,  2.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
      2.0f, 0.0f,  4.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,
      2.0f, 3.0f,  4.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,
      4.0f, 3.0f,  4.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,
      4.0f, 0.0f,  4.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,
    };

    GLuint indices[] = {
        // Surface
        0, 1, 2, 2, 3, 0,

        // Building 1
        4, 5, 6, 6, 7, 4,
        4, 5, 9, 9, 8, 4,
        5, 6, 10, 10, 9, 5,
        6, 7, 11, 11, 10, 6,
        7, 4, 8, 8, 11, 7,
        8, 9, 10, 10, 11, 8,

        // Building 2
        12, 13, 14, 14, 15, 12,
        12, 13, 17, 17, 16, 12,
        13, 14, 18, 18, 17, 13,
        14, 15, 19, 19, 18, 14,
        15, 12, 16, 16, 19, 15,
        16, 17, 18, 18, 19, 16,
    };

    // Generate and bind VAO, VBO, and EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Create and compile the shaders
    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    // Load and create a texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image, create texture, and generate mipmaps
    int width, height, nrChannels;
    unsigned char* data = stbi_load("res_wall_01_color.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Define transformations
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint lightOnLoc = glGetUniformLocation(shaderProgram, "lightOn");

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set initial light state
    bool lightOn = true;
    glUniform1i(lightOnLoc, lightOn);

    // Initialize camera
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Camera controls
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(GLFW_KEY_W, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(GLFW_KEY_S, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(GLFW_KEY_A, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(GLFW_KEY_D, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(GLFW_KEY_Q, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.ProcessKeyboard(GLFW_KEY_E, deltaTime);

        // Toggle light
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            lightOn = !lightOn;
            glUniform1i(lightOnLoc, lightOn);
        }

        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
