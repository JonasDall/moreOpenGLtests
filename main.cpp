#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>

#include "libraries/glad.h"
#include "libraries/glfw3.h"

const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char *fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\0";

enum State { release, vertex, line };

struct coordinate
{
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
};

std::vector<coordinate> triangles;
coordinate tempVertex;
coordinate tempLine;
coordinate tempTriangle;
State CurrentState = release;
unsigned int StaticVBO;
unsigned int DynamicVBO;
unsigned int StaticVAO;

coordinate pixelToWindow(float x, float y, GLFWwindow* window)
{
    int windowX, windowY;
    glfwGetWindowSize(window, &windowX, &windowY);

    coordinate returnValue;
    returnValue.x = (x / static_cast<float>(windowX) - 0.5) * 2;
    returnValue.y = (y / static_cast<float>(windowY) - 0.5) * -2;

    return returnValue;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mousex, mousey;
        glfwGetCursorPos(window, &mousex, &mousey);

        coordinate converted = pixelToWindow(mousex, mousey, window);

        std::cout << converted.x << " " << converted.y << '\n';
        std::cout << CurrentState << '\n';

        switch(CurrentState)
        {
            case release:
            tempVertex = converted;
            CurrentState = vertex;
            break;

            case vertex:
            tempLine = converted;
            CurrentState = line;
            break;

            case line:
            tempTriangle = converted;
            triangles.insert(triangles.end(), {tempVertex, tempLine, tempTriangle});
            glBindBuffer(GL_ARRAY_BUFFER, StaticVBO);
            glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(coordinate), &triangles[0], GL_DYNAMIC_DRAW);
            
            CurrentState = release;
            break;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    switch(CurrentState)
    {
        case vertex:
        tempLine = pixelToWindow(static_cast<float>(x), static_cast<float>(y), window);
        break;

        case line:
        tempTriangle = pixelToWindow(static_cast<float>(x), static_cast<float>(y), window);
        break;
    }
}

int main()
{
    //Set seed
    srand(time(0));

    //Init GLFW
    if (!glfwInit())
    {
        std::cout << "Not working!\n";
        return -1;
    }

    //Create window and set context
    GLFWwindow* window = glfwCreateWindow(500, 500, "Test", NULL, NULL);
    if (!window)
    {
        std::cout << "No window\n";
        return -1;
    }
    glfwMakeContextCurrent(window);

    //Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to load GLAD\n";
        glfwTerminate();
        return -1;
    }
    
    glGenVertexArrays(1, &StaticVAO);

    glGenBuffers(1, &StaticVBO);
    glBindBuffer(GL_ARRAY_BUFFER, StaticVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(coordinate), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &DynamicVBO);

    //Set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // glfwSetCursorPosCallback(window, cursor_position_callback);

    int success;
    char infoLog[512];

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" <<
        infoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" <<
        infoLog << std::endl;
    }

    unsigned int shader;
    shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    glGetProgramiv(shader, GL_LINK_STATUS, &success);

    if(!success) {
        glGetProgramInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINK_FAILED\n" <<
        infoLog << std::endl;
    }


    glUseProgram(shader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glBindBuffer(GL_ARRAY_BUFFER, DynamicVBO);

    glClearColor(0.2, 0.2, 0.2, 1.0);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);
        
        glBindVertexArray(StaticVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        switch(CurrentState)
        {
            case release:
            break;

            case vertex:
            break;

            case line:
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}