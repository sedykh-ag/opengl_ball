#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 10.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// camera settings
bool firstMouse = true;
float yaw   = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov   = 45.0f;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

float sphereRadius = 0.7f;

static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static void GLCheckError()
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ")" << std::endl;
    }
}

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) 
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) 
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Trajectory", NULL, NULL); // 640 x 480 is default
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);

    /* Initialize glew */
    if (glewInit() != GLEW_OK)
        std::cout << "Error" << std::endl;

    /* Print GL version */
    std::cout << glGetString(GL_VERSION) << std::endl;

    /* Shader creation and linking */
    // pink
    ShaderProgramSource sourcePink = ParseShader("res/shaders/BasicPink.shader");
    unsigned int shaderPink = CreateShader(sourcePink.VertexSource, sourcePink.FragmentSource);
    // sphere
    ShaderProgramSource sourceSphere = ParseShader("res/shaders/BasicSphere.shader");
    unsigned int shaderSphere = CreateShader(sourceSphere.VertexSource, sourceSphere.FragmentSource);


    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);

    // sphere 
    float sphere_coords[] = {
        0, 1, 0, 0, 0,
0, 1, 0, 0.1, 0,
0, 1, 0, 0.2, 0,
-0, 1, 0, 0.3, 0,
-0, 1, 0, 0.4, 0,
-0, 1, 0, 0.5, 0,
-0, 1, -0, 0.6, 0,
-0, 1, -0, 0.7, 0,
0, 1, -0, 0.8, 0,
0, 1, -0, 0.9, 0,
0, 1, -0, 1, 0,
0.309017, 0.951057, 0, 0, 0.1,
0.25, 0.951057, 0.181636, 0.1, 0.1,
0.0954915, 0.951057, 0.293893, 0.2, 0.1,
-0.0954915, 0.951057, 0.293893, 0.3, 0.1,
-0.25, 0.951057, 0.181636, 0.4, 0.1,
-0.309017, 0.951057, 3.78437e-17, 0.5, 0.1,
-0.25, 0.951057, -0.181636, 0.6, 0.1,
-0.0954915, 0.951057, -0.293893, 0.7, 0.1,
0.0954915, 0.951057, -0.293893, 0.8, 0.1,
0.25, 0.951057, -0.181636, 0.9, 0.1,
0.309017, 0.951057, -7.56873e-17, 1, 0.1,
0.587785, 0.809017, 0, 0, 0.2,
0.475528, 0.809017, 0.345491, 0.1, 0.2,
0.181636, 0.809017, 0.559017, 0.2, 0.2,
-0.181636, 0.809017, 0.559017, 0.3, 0.2,
-0.475528, 0.809017, 0.345491, 0.4, 0.2,
-0.587785, 0.809017, 7.19829e-17, 0.5, 0.2,
-0.475528, 0.809017, -0.345492, 0.6, 0.2,
-0.181636, 0.809017, -0.559017, 0.7, 0.2,
0.181636, 0.809017, -0.559017, 0.8, 0.2,
0.475528, 0.809017, -0.345492, 0.9, 0.2,
0.587785, 0.809017, -1.43966e-16, 1, 0.2,
0.809017, 0.587785, 0, 0, 0.3,
0.654509, 0.587785, 0.475528, 0.1, 0.3,
0.25, 0.587785, 0.769421, 0.2, 0.3,
-0.25, 0.587785, 0.769421, 0.3, 0.3,
-0.654509, 0.587785, 0.475528, 0.4, 0.3,
-0.809017, 0.587785, 9.9076e-17, 0.5, 0.3,
-0.654508, 0.587785, -0.475528, 0.6, 0.3,
-0.25, 0.587785, -0.769421, 0.7, 0.3,
0.25, 0.587785, -0.769421, 0.8, 0.3,
0.654508, 0.587785, -0.475528, 0.9, 0.3,
0.809017, 0.587785, -1.98152e-16, 1, 0.3,
0.951057, 0.309017, 0, 0, 0.4,
0.769421, 0.309017, 0.559017, 0.1, 0.4,
0.293893, 0.309017, 0.904509, 0.2, 0.4,
-0.293893, 0.309017, 0.904508, 0.3, 0.4,
-0.769421, 0.309017, 0.559017, 0.4, 0.4,
-0.951057, 0.309017, 1.16471e-16, 0.5, 0.4,
-0.769421, 0.309017, -0.559017, 0.6, 0.4,
-0.293893, 0.309017, -0.904508, 0.7, 0.4,
0.293893, 0.309017, -0.904508, 0.8, 0.4,
0.769421, 0.309017, -0.559017, 0.9, 0.4,
0.951057, 0.309017, -2.32942e-16, 1, 0.4,
1, 6.12323e-17, 0, 0, 0.5,
0.809017, 6.12323e-17, 0.587785, 0.1, 0.5,
0.309017, 6.12323e-17, 0.951057, 0.2, 0.5,
-0.309017, 6.12323e-17, 0.951056, 0.3, 0.5,
-0.809017, 6.12323e-17, 0.587785, 0.4, 0.5,
-1, 6.12323e-17, 1.22465e-16, 0.5, 0.5,
-0.809017, 6.12323e-17, -0.587785, 0.6, 0.5,
-0.309017, 6.12323e-17, -0.951056, 0.7, 0.5,
0.309017, 6.12323e-17, -0.951056, 0.8, 0.5,
0.809017, 6.12323e-17, -0.587785, 0.9, 0.5,
1, 6.12323e-17, -2.44929e-16, 1, 0.5,
0.951056, -0.309017, 0, 0, 0.6,
0.769421, -0.309017, 0.559017, 0.1, 0.6,
0.293893, -0.309017, 0.904508, 0.2, 0.6,
-0.293893, -0.309017, 0.904508, 0.3, 0.6,
-0.769421, -0.309017, 0.559017, 0.4, 0.6,
-0.951056, -0.309017, 1.16471e-16, 0.5, 0.6,
-0.769421, -0.309017, -0.559017, 0.6, 0.6,
-0.293893, -0.309017, -0.904508, 0.7, 0.6,
0.293893, -0.309017, -0.904508, 0.8, 0.6,
0.769421, -0.309017, -0.559017, 0.9, 0.6,
0.951056, -0.309017, -2.32942e-16, 1, 0.6,
0.809017, -0.587785, 0, 0, 0.7,
0.654509, -0.587785, 0.475528, 0.1, 0.7,
0.25, -0.587785, 0.769421, 0.2, 0.7,
-0.25, -0.587785, 0.769421, 0.3, 0.7,
-0.654509, -0.587785, 0.475528, 0.4, 0.7,
-0.809017, -0.587785, 9.9076e-17, 0.5, 0.7,
-0.654508, -0.587785, -0.475528, 0.6, 0.7,
-0.25, -0.587785, -0.769421, 0.7, 0.7,
0.25, -0.587785, -0.769421, 0.8, 0.7,
0.654508, -0.587785, -0.475528, 0.9, 0.7,
0.809017, -0.587785, -1.98152e-16, 1, 0.7,
0.587785, -0.809017, 0, 0, 0.8,
0.475528, -0.809017, 0.345491, 0.1, 0.8,
0.181636, -0.809017, 0.559017, 0.2, 0.8,
-0.181636, -0.809017, 0.559017, 0.3, 0.8,
-0.475528, -0.809017, 0.345491, 0.4, 0.8,
-0.587785, -0.809017, 7.19829e-17, 0.5, 0.8,
-0.475528, -0.809017, -0.345492, 0.6, 0.8,
-0.181636, -0.809017, -0.559017, 0.7, 0.8,
0.181636, -0.809017, -0.559017, 0.8, 0.8,
0.475528, -0.809017, -0.345492, 0.9, 0.8,
0.587785, -0.809017, -1.43966e-16, 1, 0.8,
0.309017, -0.951056, 0, 0, 0.9,
0.25, -0.951056, 0.181636, 0.1, 0.9,
0.0954915, -0.951056, 0.293893, 0.2, 0.9,
-0.0954915, -0.951056, 0.293893, 0.3, 0.9,
-0.25, -0.951056, 0.181636, 0.4, 0.9,
-0.309017, -0.951056, 3.78437e-17, 0.5, 0.9,
-0.25, -0.951056, -0.181636, 0.6, 0.9,
-0.0954915, -0.951056, -0.293893, 0.7, 0.9,
0.0954915, -0.951056, -0.293893, 0.8, 0.9,
0.25, -0.951056, -0.181636, 0.9, 0.9,
0.309017, -0.951056, -7.56874e-17, 1, 0.9,
1.22465e-16, -1, 0, 0, 1,
9.9076e-17, -1, 7.19829e-17, 0.1, 1,
3.78437e-17, -1, 1.16471e-16, 0.2, 1,
-3.78437e-17, -1, 1.16471e-16, 0.3, 1,
-9.9076e-17, -1, 7.19829e-17, 0.4, 1,
-1.22465e-16, -1, 1.49976e-32, 0.5, 1,
-9.9076e-17, -1, -7.19829e-17, 0.6, 1,
-3.78437e-17, -1, -1.16471e-16, 0.7, 1,
3.78437e-17, -1, -1.16471e-16, 0.8, 1,
9.9076e-17, -1, -7.19829e-17, 0.9, 1,
1.22465e-16, -1, -2.99952e-32, 1, 1
    }; // now with UV
    unsigned int sphere_indices[]{
        11, 0, 1, 11, 1, 12,
12, 1, 2, 12, 2, 13,
13, 2, 3, 13, 3, 14,
14, 3, 4, 14, 4, 15,
15, 4, 5, 15, 5, 16,
16, 5, 6, 16, 6, 17,
17, 6, 7, 17, 7, 18,
18, 7, 8, 18, 8, 19,
19, 8, 9, 19, 9, 20,
20, 9, 10, 20, 10, 21,
22, 11, 12, 22, 12, 23,
23, 12, 13, 23, 13, 24,
24, 13, 14, 24, 14, 25,
25, 14, 15, 25, 15, 26,
26, 15, 16, 26, 16, 27,
27, 16, 17, 27, 17, 28,
28, 17, 18, 28, 18, 29,
29, 18, 19, 29, 19, 30,
30, 19, 20, 30, 20, 31,
31, 20, 21, 31, 21, 32,
33, 22, 23, 33, 23, 34,
34, 23, 24, 34, 24, 35,
35, 24, 25, 35, 25, 36,
36, 25, 26, 36, 26, 37,
37, 26, 27, 37, 27, 38,
38, 27, 28, 38, 28, 39,
39, 28, 29, 39, 29, 40,
40, 29, 30, 40, 30, 41,
41, 30, 31, 41, 31, 42,
42, 31, 32, 42, 32, 43,
44, 33, 34, 44, 34, 45,
45, 34, 35, 45, 35, 46,
46, 35, 36, 46, 36, 47,
47, 36, 37, 47, 37, 48,
48, 37, 38, 48, 38, 49,
49, 38, 39, 49, 39, 50,
50, 39, 40, 50, 40, 51,
51, 40, 41, 51, 41, 52,
52, 41, 42, 52, 42, 53,
53, 42, 43, 53, 43, 54,
55, 44, 45, 55, 45, 56,
56, 45, 46, 56, 46, 57,
57, 46, 47, 57, 47, 58,
58, 47, 48, 58, 48, 59,
59, 48, 49, 59, 49, 60,
60, 49, 50, 60, 50, 61,
61, 50, 51, 61, 51, 62,
62, 51, 52, 62, 52, 63,
63, 52, 53, 63, 53, 64,
64, 53, 54, 64, 54, 65,
66, 55, 56, 66, 56, 67,
67, 56, 57, 67, 57, 68,
68, 57, 58, 68, 58, 69,
69, 58, 59, 69, 59, 70,
70, 59, 60, 70, 60, 71,
71, 60, 61, 71, 61, 72,
72, 61, 62, 72, 62, 73,
73, 62, 63, 73, 63, 74,
74, 63, 64, 74, 64, 75,
75, 64, 65, 75, 65, 76,
77, 66, 67, 77, 67, 78,
78, 67, 68, 78, 68, 79,
79, 68, 69, 79, 69, 80,
80, 69, 70, 80, 70, 81,
81, 70, 71, 81, 71, 82,
82, 71, 72, 82, 72, 83,
83, 72, 73, 83, 73, 84,
84, 73, 74, 84, 74, 85,
85, 74, 75, 85, 75, 86,
86, 75, 76, 86, 76, 87,
88, 77, 78, 88, 78, 89,
89, 78, 79, 89, 79, 90,
90, 79, 80, 90, 80, 91,
91, 80, 81, 91, 81, 92,
92, 81, 82, 92, 82, 93,
93, 82, 83, 93, 83, 94,
94, 83, 84, 94, 84, 95,
95, 84, 85, 95, 85, 96,
96, 85, 86, 96, 86, 97,
97, 86, 87, 97, 87, 98,
99, 88, 89, 99, 89, 100,
100, 89, 90, 100, 90, 101,
101, 90, 91, 101, 91, 102,
102, 91, 92, 102, 92, 103,
103, 92, 93, 103, 93, 104,
104, 93, 94, 104, 94, 105,
105, 94, 95, 105, 95, 106,
106, 95, 96, 106, 96, 107,
107, 96, 97, 107, 97, 108,
108, 97, 98, 108, 98, 109,
110, 99, 100, 110, 100, 111,
111, 100, 101, 111, 101, 112,
112, 101, 102, 112, 102, 113,
113, 102, 103, 113, 103, 114,
114, 103, 104, 114, 104, 115,
115, 104, 105, 115, 105, 116,
116, 105, 106, 116, 106, 117,
117, 106, 107, 117, 107, 118,
118, 107, 108, 118, 108, 119,
119, 108, 109, 119, 109, 120
    };
    unsigned int VAO_sphere, VBO_sphere, EBO_sphere;
    glGenVertexArrays(1, &VAO_sphere);
    glGenBuffers(1, &VBO_sphere);
    glGenBuffers(1, &EBO_sphere);
    glBindVertexArray(VAO_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_sphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_sphere);

    /* Texture source */
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("res/textures/mars.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_coords), sphere_coords, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // floor surface
    float floor_coords[] = {
        -1000.0f, -50.0f,  1000.0f,
         1000.0f, -50.0f,  1000.0f,
         1000.0f, -50.0f, -1000.0f,
                 
         1000.0f, -50.0f, -1000.0f,
        -1000.0f, -50.0f, -1000.0f,
        -1000.0f, -50.0f,  1000.0f
    };
    unsigned int VAO_floor, VBO_floor;
    glGenVertexArrays(1, &VAO_floor);
    glGenBuffers(1, &VBO_floor);
    glBindVertexArray(VAO_floor);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_coords), floor_coords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glBindVertexArray(0);

    // trajectory
    unsigned int VAO_trajectory, VBO_trajectory;
    glGenVertexArrays(1, &VAO_trajectory);
    glGenBuffers(1, &VBO_trajectory);
    glBindVertexArray(VAO_trajectory);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_trajectory);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glBindVertexArray(0);

    // trajectory_nf
    unsigned int VAO_trajectory_nf, VBO_trajectory_nf;
    glGenVertexArrays(1, &VAO_trajectory_nf);
    glGenBuffers(1, &VBO_trajectory_nf);
    glBindVertexArray(VAO_trajectory_nf);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_trajectory_nf);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glBindVertexArray(0);


    // physics
    std::vector<float> trajectory_coords;
    std::vector<float> trajectory_nf_coords; // nf = no friction
    glm::vec3 positions = glm::vec3(0.0f, 10.0f, 0.0f);
    glm::vec3 velocity = glm::vec3(5.0f, 0.0f, 0.0f);
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 positions_nf = glm::vec3(0.0f, 10.0f, 0.0f);
    glm::vec3 velocity_nf = glm::vec3(5.0f, 0.0f, 0.0f);
    glm::vec3 acceleration_nf = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 g_accel = glm::vec3(0.0f, -5.0f, 0.0f);
    const float beta = 0.5f;
    const float mass = 1.0f;
    const float k = beta / mass;


    unsigned int trajectory_count = 0;

    // projection matrix
    glm::mat4 projection = glm::perspective(fov, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glUseProgram(shaderPink);
    glUniformMatrix4fv(glGetUniformLocation(shaderPink, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUseProgram(shaderSphere);
    glUniformMatrix4fv(glGetUniformLocation(shaderSphere, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    // model for trajectory
    glm::mat4 model = glm::mat4(1.0f);
    glUseProgram(shaderPink);
    glUniformMatrix4fv(glGetUniformLocation(shaderPink, "model"), 1, GL_FALSE, glm::value_ptr(model));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // physics
        glm::vec3 fricton_accel = -k * velocity;
        acceleration = fricton_accel + g_accel;
        velocity += deltaTime * acceleration;
        positions += deltaTime * velocity;
        // nf
        acceleration_nf = g_accel;
        velocity_nf += deltaTime * acceleration_nf;
        positions_nf += deltaTime * velocity_nf;

        if (trajectory_count < 1000000) {
            trajectory_coords.push_back(positions.x);
            trajectory_coords.push_back(positions.y);
            trajectory_coords.push_back(positions.z);

            trajectory_nf_coords.push_back(positions_nf.x);
            trajectory_nf_coords.push_back(positions_nf.y);
            trajectory_nf_coords.push_back(positions_nf.z);

            trajectory_count++;
        }
        

        /* Input */
        processInput(window);

        // model for sphere
        glm::mat4 model_sphere = glm::mat4(1.0f);
        model_sphere = glm::translate(model_sphere, positions);
        model_sphere = glm::rotate(model_sphere, (float)glfwGetTime() * glm::radians(180.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        model_sphere = glm::scale(model_sphere, glm::vec3(sphereRadius));
        glUseProgram(shaderSphere);
        glUniformMatrix4fv(glGetUniformLocation(shaderSphere, "model"), 1, GL_FALSE, glm::value_ptr(model_sphere));

        // view
        cameraPos = glm::vec3(positions.x - 1.0f, positions.y + 10.0f, 5.0f + (float)glfwGetTime() * 2.0f);
        glm::mat4 view = glm::lookAt(cameraPos, positions, cameraUp);
        glUseProgram(shaderPink);
        glUniformMatrix4fv(glGetUniformLocation(shaderPink, "view"), 1, GL_FALSE, &view[0][0]);
        glUseProgram(shaderSphere);
        glUniformMatrix4fv(glGetUniformLocation(shaderSphere, "view"), 1, GL_FALSE, &view[0][0]);
        


        /* Render here */
        glClearColor(0.6f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw floor
        glBindVertexArray(VAO_floor);
        glUseProgram(shaderPink);
        glUniform4f(glGetUniformLocation(shaderPink, "ourColor"), 0.3f, 0.3f, 0.3f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // draw trajectory
        glBindVertexArray(VAO_trajectory);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_trajectory);
        glBufferData(GL_ARRAY_BUFFER, trajectory_coords.size() * sizeof(float), &trajectory_coords[0], GL_DYNAMIC_DRAW);
        glUseProgram(shaderPink);
        glUniform4f(glGetUniformLocation(shaderPink, "ourColor"), 0.0f, 0.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINE_STRIP, 0, trajectory_count);
        glBindVertexArray(0);

        // draw trajectory_nf
        glBindVertexArray(VAO_trajectory_nf);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_trajectory_nf);
        glBufferData(GL_ARRAY_BUFFER, trajectory_nf_coords.size() * sizeof(float), &trajectory_nf_coords[0], GL_DYNAMIC_DRAW);
        glUseProgram(shaderPink);
        glUniform4f(glGetUniformLocation(shaderPink, "ourColor"), 0.87f, 0.2f, 0.84f, 1.0f); // pink
        glDrawArrays(GL_LINE_STRIP, 0, trajectory_count);
        glBindVertexArray(0);

        // draw sphere
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO_sphere);
        glUseProgram(shaderSphere);
        glDrawElements(GL_TRIANGLES, sizeof(sphere_indices), GL_UNSIGNED_INT, 0);
        

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shaderPink);
    glDeleteProgram(shaderSphere);

    glfwTerminate();
    return 0;
};

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{   
    float sensitivity = 0.1f;
    fov -= (float)yoffset * sensitivity;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}