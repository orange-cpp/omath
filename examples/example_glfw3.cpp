// main.cpp
#include <cstdint>
#include <iostream>
#include <vector>

// --- OpenGL / windowing ---
#include <GL/glew.h> // GLEW must come before GLFW
#include <GLFW/glfw3.h>

// --- your math / engine stuff ---
#include "omath/3d_primitives/mesh.hpp"
#include "omath/engines/opengl_engine/camera.hpp"
#include "omath/engines/opengl_engine/constants.hpp"
#include "omath/engines/opengl_engine/mesh.hpp"
#include "omath/linear_algebra/vector3.hpp"

using omath::Vector3;

// ---------------- TYPE ALIASES (ADAPT TO YOUR LIB) ----------------

// Your 4x4 matrix type
using Mat4x4 = omath::opengl_engine::Mat4X4;

// Rotation angles for the Mesh (whatever you use for rotation)
using RotationAngles =
        omath::opengl_engine::ViewAngles; // TODO: if you have a dedicated angle type for mesh rotation, use it

// View angles type for camera trait
using ViewAngles = RotationAngles; // TODO: if your camera uses a different view-angle type, change this

// Your trait types (that satisfy the concepts declared in your headers)
using CameraTrait = /* TODO: your camera engine trait type here */ void;
using MeshTrait = /* TODO: your mesh rotation trait type here */ void;

// For brevity, alias the templates instantiated with your types
using VertexType = omath::primitives::Vertex<Vector3<float>>;
using CubeMesh = omath::opengl_engine::Mesh;
using MyCamera = omath::opengl_engine::Camera;

// ---------------- SHADERS ----------------

static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aUv;

uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vNormal;
out vec3 vUv;

void main() {
    vNormal = aNormal;
    vUv = aUv;
    gl_Position = uMVP * uModel * vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vNormal;
in vec3 vUv;

out vec4 FragColor;

void main() {
    vec3 baseColor = normalize(abs(vNormal));
    FragColor = vec4(baseColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return shader;
}

GLuint createShaderProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int w, int h)
{
    glViewport(0, 0, w, h);
}

// ---------------- MAIN ----------------

int main()
{
    // ---------- GLFW init ----------
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    const int SCR_WIDTH = 800;
    const int SCR_HEIGHT = 600;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "omath cube + camera (GLEW)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ---------- GLEW init ----------
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW: " << reinterpret_cast<const char*>(glewGetErrorString(glewErr))
                  << "\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // ---------- Build Cube Mesh (CPU side) ----------
    std::vector<omath::primitives::Vertex<>> vbo;
    vbo.reserve(8);

    Vector3<float> p000{-0.5f, -0.5f, -0.5f};
    Vector3<float> p001{-0.5f, -0.5f, 0.5f};
    Vector3<float> p010{-0.5f, 0.5f, -0.5f};
    Vector3<float> p011{-0.5f, 0.5f, 0.5f};
    Vector3<float> p100{0.5f, -0.5f, -0.5f};
    Vector3<float> p101{0.5f, -0.5f, 0.5f};
    Vector3<float> p110{0.5f, 0.5f, -0.5f};
    Vector3<float> p111{0.5f, 0.5f, 0.5f};

    VertexType v0{p000, Vector3<float>{-1, -1, -1}, Vector3<float>{0, 0, 0}};
    VertexType v1{p001, Vector3<float>{-1, -1, 1}, Vector3<float>{0, 1, 0}};
    VertexType v2{p010, Vector3<float>{-1, 1, -1}, Vector3<float>{1, 0, 0}};
    VertexType v3{p011, Vector3<float>{-1, 1, 1}, Vector3<float>{1, 1, 0}};
    VertexType v4{p100, Vector3<float>{1, -1, -1}, Vector3<float>{0, 0, 1}};
    VertexType v5{p101, Vector3<float>{1, -1, 1}, Vector3<float>{0, 1, 1}};
    VertexType v6{p110, Vector3<float>{1, 1, -1}, Vector3<float>{1, 0, 1}};
    VertexType v7{p111, Vector3<float>{1, 1, 1}, Vector3<float>{1, 1, 1}};

    vbo.push_back(v0); // 0
    vbo.push_back(v1); // 1
    vbo.push_back(v2); // 2
    vbo.push_back(v3); // 3
    vbo.push_back(v4); // 4
    vbo.push_back(v5); // 5
    vbo.push_back(v6); // 6
    vbo.push_back(v7); // 7

    std::vector<Vector3<std::uint32_t>> ebo;
    ebo.reserve(12);
    using Idx = Vector3<std::uint32_t>;

    // front (z+)
    ebo.emplace_back(Idx{1, 5, 7});
    ebo.emplace_back(Idx{1, 7, 3});

    // back (z-)
    ebo.emplace_back(Idx{0, 2, 6});
    ebo.emplace_back(Idx{0, 6, 4});

    // left (x-)
    ebo.emplace_back(Idx{0, 1, 3});
    ebo.emplace_back(Idx{0, 3, 2});

    // right (x+)
    ebo.emplace_back(Idx{4, 6, 7});
    ebo.emplace_back(Idx{4, 7, 5});

    // bottom (y-)
    ebo.emplace_back(Idx{0, 4, 5});
    ebo.emplace_back(Idx{0, 5, 1});

    // top (y+)
    ebo.emplace_back(Idx{2, 3, 7});
    ebo.emplace_back(Idx{2, 7, 6});

    CubeMesh cube{std::move(vbo), std::move(ebo)};
    cube.set_origin(Vector3<float>{0.f, 0.f, 0.f});
    cube.set_scale(Vector3<float>{1.f, 2.f, 1.f});
    cube.set_rotation(RotationAngles{});

    // ---------- OpenGL buffers ----------
    GLuint VAO = 0, VBO = 0, EBO_GL = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO_GL);

    glBindVertexArray(VAO);

    // upload vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cube.m_vertex_buffer.size() * sizeof(VertexType), cube.m_vertex_buffer.data(),
                 GL_STATIC_DRAW);

    // flatten Ebo to GL indices
    std::vector<GLuint> flatIndices;
    flatIndices.reserve(cube.m_vertex_array_object.size() * 3);
    for (const auto& tri : cube.m_vertex_array_object)
    {
        flatIndices.push_back(tri.x);
        flatIndices.push_back(tri.y);
        flatIndices.push_back(tri.z);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_GL);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(GLuint), flatIndices.data(), GL_STATIC_DRAW);

    // vertex layout: position / normal / uv (each Vector3<float>)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, uv));

    glBindVertexArray(0);

    // ---------- Camera setup ----------
    omath::projection::ViewPort viewPort{static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT)};

    Vector3<float> camPos{0.f, 0.f, 3.f};

    float nearPlane = 0.1f;
    float farPlane = 100.f;
    auto fov = omath::projection::FieldOfView::from_degrees(90.f);
    // NOTE: you must replace CameraTrait alias above with your real type
    MyCamera camera{camPos, {}, viewPort, fov, nearPlane, farPlane};

    // ---------- Shader ----------
    GLuint shaderProgram = createShaderProgram();
    GLint uMvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    GLint uModel = glGetUniformLocation(shaderProgram, "uModel");
    static float old_frame_time = glfwGetTime();

    // ---------- Main loop ----------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - old_frame_time;
        old_frame_time = currentTime;
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);

        viewPort.m_width = static_cast<float>(fbW);
        viewPort.m_height = static_cast<float>(fbH);
        camera.set_view_port(viewPort);

        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RotationAngles rot = cube.get_rotation_angles();
        rot.yaw += omath::opengl_engine::YawAngle::from_degrees(40.f * deltaTime);
        rot.roll += omath::opengl_engine::RollAngle::from_degrees(40.f * deltaTime);
        rot.pitch += omath::opengl_engine::PitchAngle::from_degrees(40.f * deltaTime);
        cube.set_rotation(rot);

        const Mat4x4& viewProj = camera.get_view_projection_matrix();
        const auto& model = cube.get_to_world_matrix();
        glUseProgram(shaderProgram);

        // Send matrix to GPU
        // TODO: replace mvp.data() with whatever your Mat4x4 uses to expose float*
        const float* mvpPtr = viewProj.raw_array().data(); // assumes column-major float[16]

        glUniformMatrix4fv(uMvpLoc, 1, GL_FALSE, mvpPtr);
        const float* modelPtr = model.raw_array().data(); // assumes column-major float[16]
        glBindVertexArray(VAO);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, modelPtr);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(flatIndices.size()), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
    }

    // ---------- Cleanup ----------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO_GL);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}