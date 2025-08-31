#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

// Vertex Shader
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)";

// Fragment Shader
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow *window = glfwCreateWindow(800, 800, "OpenGL Circle", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL function pointers
    if (!gladLoadGL())
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, 800, 800);

    // Circle parameters (normalized device coordinates)
    const int numPoints = 30;
    float radius = 0.1f;
    std::vector<GLfloat> vertices;
    

    float circleVelocityX = 0.01f;
    float circleVelocityY = 0.01f;

    float centerPositionX = 0.0f;
    float centerPositionY = 0.0f;

    // Center vertex
    vertices.push_back(0.0f + centerPositionX);
    vertices.push_back(0.0f + centerPositionY);
    vertices.push_back(0.0f);

    // Circle perimeter vertices
    for (int i = 0; i <= numPoints; ++i)
    {
        float angle = 2.0f * M_PI * i / numPoints;
        vertices.push_back((cosf(angle) * radius) + centerPositionX);
        vertices.push_back((sinf(angle) * radius) + centerPositionY);
        vertices.push_back(0.0f);
    }

    // Indices to draw triangle fan
    std::vector<GLuint> indices;
    for (int i = 1; i <= numPoints; ++i)
    {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // Shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Buffers
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Don't unbind EBO separately!

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {

        // Circle perimeter vertices
        for (int i = 0; i <= numPoints; ++i)
        {
            float angle = 2.0f * M_PI * i / numPoints;
            vertices.push_back((cosf(angle) * radius) + centerPositionX);
            vertices.push_back((sinf(angle) * radius) + centerPositionY);
            vertices.push_back(0.0f);
        }

        glBindVertexArray(VAO);
    
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0); // Don't unbind EBO separately!



        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (centerPositionX > 1.0f)
        {
            circleVelocityX = -0.1f;
        }
        if (centerPositionY > 1.0f)
        {
            circleVelocityY = -0.1f;
        }
        if (centerPositionX < -1.0f)
        {
            circleVelocityX = 0.1f;
        }
        if (centerPositionY < -1.0f)
        {
            circleVelocityY = 0.1f;
        }
        
        centerPositionX += circleVelocityX;
        centerPositionY += circleVelocityY;

        // std::cout << centerPositionX << "\n";
        // std::cout << centerPositionY << "\n";
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
