#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// keep this header in front of glfw
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const GLchar *vertexShaderSource = "#version 330 core\n"
                                   "layout (location = 0) in vec3 position;\n"
                                   "layout (location = 1) in vec3 color;\n"
                                   "out vec3 ourcolor;\n"
                                   "void main()\n"
                                   "{\n"
                                   " gl_Position = vec4(position,1.0);\n"
                                   " ourcolor = color;\n"
                                   "}\0";

const GLchar *fragmentShaderSource = "#version 330 core\n"
                                     "in vec3 ourcolor;\n"
                                     "out vec4 color;\n"
                                     "void main()\n"
                                     "{\n"
                                     " color = vec4(ourcolor,1.0f);\n"
                                     "}\n\0";

int main() {
    // glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "cs334 hello world", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create glfw window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLEW load all opengl function pointers
    if (glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    //
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }
    // link shaders
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    //
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //
    float vertices[] = {
        // Positions         // Colors
        0.0f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // 1 Bottom Right
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // 1 Bottom Left
        0.0f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, // 1 Top Right
        -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, // 1 Top Left
        0.0f,  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, // 2 Bottom Right
        -0.5f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f, // 2 Bottom Left
        0.3f,  0.3f,  0.0f, 0.0f, 0.0f, 1.0f, // 2 Top Right
        -0.2f, 0.3f,  0.0f, 0.0f, 0.0f, 1.0f, // 2 Top Left
        0.3f,  -0.2f, 0.0f, 0.0f, 1.0f, 0.0f, // 3 Bottom Right
        0.0f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // 3 Bottom Left
        0.3f,  0.3f,  0.0f, 0.0f, 1.0f, 0.0f, // 3 Top Right
        0.0f,  0.0f,  0.0f, 0.0f, 1.0f, 0.0f  // 3 Top Left
    };

    //
    unsigned int indices[] = {
        // note that we start from 0!
        0, 1,  3,  // 1 Triangle
        0, 2,  3,  // 2 Triangle
        4, 5,  7,  // 3 Triangle
        4, 6,  7,  // 4 Triangle
        8, 9,  11, // 5 Triangle
        8, 10, 11  // 6 Triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    //
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // unbind VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // unbind VAO
    glBindVertexArray(0);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn
        // more about Dear ImGui!).

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            // ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES,0,6);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// 处理键盘输入
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }