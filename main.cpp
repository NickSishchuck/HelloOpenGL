#define GLFW_INCLUDE_NONE
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <E:\Programming/OpenGL\HelloOpenGL\glfw-3.4\deps\linmath.h> // Please include linmath using your own explorer path
#include <iostream>
#include <cstdlib>   // For std::exit, EXIT_FAILURE
#include <cstdio>
#include <glbinding/Binding.h>
#include <glbinding/Version.h>



// To avoid typing gl:: everywhere
using namespace gl;

// A simple struct holding 2D position + 3D color
struct Vertex
{
    vec2 pos;
    vec3 col;
};

static const Vertex vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } }, // Red
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } }, // Green
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }  // Blue
};

static const char* vertex_shader_text =
R"(
#version 330
uniform mat4 MVP;
in vec3 vCol;
in vec2 vPos;
out vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

static const char* fragment_shader_text =
R"(
#version 330
in vec3 color;
out vec4 fragment;
void main()
{
    fragment = vec4(color, 1.0);
}
)";

// Error callback for GLFW
static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

// Key callback for GLFW
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
    // Initialize GLFW
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(EXIT_FAILURE);
    }

    // Request an OpenGL 3.3 Core Profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle (glbinding)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Set key callback and make the window's context current
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    // Initialize glbinding AFTER making the context current!!!
    //    This loads the actual OpenGL function pointers.
    glbinding::Binding::initialize(nullptr, false);

    // 4. V-Sync
    glfwSwapInterval(1);

    // 5. Create a Vertex Buffer Object (VBO) and upload vertex data
    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 6. Create & compile vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr);
    glCompileShader(vertex_shader);

    // 7. Create & compile fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr);
    glCompileShader(fragment_shader);

    // 8. Link shaders into a program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // 9. Get uniform and attribute locations
    GLint mvp_location = glGetUniformLocation(program, "MVP");
    GLint vpos_location = glGetAttribLocation(program, "vPos");
    GLint vcol_location = glGetAttribLocation(program, "vCol");

    // 10. Create a Vertex Array Object (VAO) to store the attribute setup
    GLuint vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    // Enable vertex position attribute
    glEnableVertexAttribArray(static_cast<GLuint>(vpos_location));
    glVertexAttribPointer(static_cast<GLuint>(vpos_location), 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, pos)));

    // Enable color attribute
    glEnableVertexAttribArray(static_cast<GLuint>(vcol_location));
    glVertexAttribPointer(static_cast<GLuint>(vcol_location), 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        reinterpret_cast<void*>(offsetof(Vertex, col)));

    // Main render loop
    while (!glfwWindowShouldClose(window))
    {
        // Get window size, compute aspect ratio
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = (height == 0) ? 1.f : (width / static_cast<float>(height));

        // Set viewport and clear
        gl::glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // Build Model-View-Projection (MVP) matrix
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, static_cast<float>(glfwGetTime()));
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        // Use the shader program and pass in the MVP
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&mvp);

        // Bind the VAO and draw
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Swap buffers, poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteVertexArrays(1, &vertex_array);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}