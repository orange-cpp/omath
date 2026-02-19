#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "omath/linear_algebra/vector3.hpp"
#include "omath/linear_algebra/triangle.hpp"

using omath::Vector3;
using omath::Triangle;

static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in float aPointSize;
layout (location = 3) in float aIsLine;

out vec3 vColor;
out float vIsLine;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vColor = aColor;
    gl_PointSize = aPointSize;
    vIsLine = aIsLine;
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vColor;
in float vIsLine;
out vec4 FragColor;

void main() {
    if (vIsLine < 0.5) {
        // Calculate distance from center of the point
        vec2 coord = gl_PointCoord - vec2(0.5);
        if(length(coord) > 0.5)
            discard;
    }
        
    FragColor = vec4(vColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader error: " << log << std::endl;
    }
    return shader;
}

void drawChar(char c, float x, float y, float scale, const Vector3<float>& color, std::vector<float>& lines) {
    float w = 0.5f * scale;
    float h = 1.0f * scale;
    
    auto add = [&](float x1, float y1, float x2, float y2) {
        lines.push_back(x + x1*w); lines.push_back(y + y1*h); lines.push_back(0.0f);
        lines.push_back(color.x); lines.push_back(color.y); lines.push_back(color.z);
        lines.push_back(1.0f); // size
        lines.push_back(1.0f); // isLine
        
        lines.push_back(x + x2*w); lines.push_back(y + y2*h); lines.push_back(0.0f);
        lines.push_back(color.x); lines.push_back(color.y); lines.push_back(color.z);
        lines.push_back(1.0f); // size
        lines.push_back(1.0f); // isLine
    };

    switch(c) {
        case '0': add(0,0, 1,0); add(1,0, 1,1); add(1,1, 0,1); add(0,1, 0,0); break;
        case '1': add(0.5f,0, 0.5f,1); add(0.25f,0.75f, 0.5f,1); add(0.25f,0, 0.75f,0); break;
        case '2': add(0,1, 1,1); add(1,1, 1,0.5f); add(1,0.5f, 0,0.5f); add(0,0.5f, 0,0); add(0,0, 1,0); break;
        case '3': add(0,1, 1,1); add(1,1, 1,0); add(1,0, 0,0); add(0,0.5f, 1,0.5f); break;
        case '4': add(0,1, 0,0.5f); add(0,0.5f, 1,0.5f); add(1,1, 1,0); break;
        case '5': add(1,1, 0,1); add(0,1, 0,0.5f); add(0,0.5f, 1,0.5f); add(1,0.5f, 1,0); add(1,0, 0,0); break;
        case '6': add(1,1, 0,1); add(0,1, 0,0); add(0,0, 1,0); add(1,0, 1,0.5f); add(1,0.5f, 0,0.5f); break;
        case '7': add(0,1, 1,1); add(1,1, 0.5f,0); break;
        case '8': add(0,0, 1,0); add(1,0, 1,1); add(1,1, 0,1); add(0,1, 0,0); add(0,0.5f, 1,0.5f); break;
        case '9': add(1,0.5f, 0,0.5f); add(0,0.5f, 0,1); add(0,1, 1,1); add(1,1, 1,0); add(1,0, 0,0); break;
        case '.': add(0.4f,0, 0.6f,0); add(0.6f,0, 0.6f,0.2f); add(0.6f,0.2f, 0.4f,0.2f); add(0.4f,0.2f, 0.4f,0); break;
    }
}

void drawText(const std::string& text, float x, float y, float scale, const Vector3<float>& color, std::vector<float>& lines) {
    float cursor = x;
    for (char c : text) {
        drawChar(c, cursor, y, scale, color, lines);
        cursor += (c == '.' ? 0.3f : 0.7f) * scale;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Barycentric Coordinates", NULL, NULL);
    if (!window) { 
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate(); 
        return -1; 
    }
    glfwMakeContextCurrent(window);

    // Check if context is valid using standard GL
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    if (renderer && version) {
        std::cout << "Renderer: " << renderer << "\n";
        std::cout << "OpenGL version supported: " << version << "\n";
    } else {
        std::cerr << "Failed to get GL_RENDERER or GL_VERSION. Context might be invalid.\n";
    }

    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        // Ignore GLEW_ERROR_NO_GLX_DISPLAY if we have a valid context (e.g. Wayland)
        if (glewErr == GLEW_ERROR_NO_GLX_DISPLAY && renderer) {
            std::cerr << "GLEW warning: " << glewGetErrorString(glewErr) << " (Ignored because context seems valid)\n";
        } else {
            std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << "\n";
            glfwTerminate();
            return -1;
        }
    }

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    // Triangle vertices as shown in the picture (Red, Green, Blue)
    // Scaled down slightly to leave room for text
    Vector3<float> A(0.0f, 0.6f, 0.0f);   // Red dot (top)
    Vector3<float> B(-0.6f, -0.6f, 0.0f); // Green dot (bottom left)
    Vector3<float> C(0.6f, -0.6f, 0.0f);  // Blue dot (bottom right)

    std::vector<float> pointCloud;
    
    // Iterating over barycentric coordinates (u, v, w) from 0.0 to 1.0
    for (float u = 0.0f; u <= 1.0f; u += 0.015f) {
        for (float v = 0.0f; v <= 1.0f - u; v += 0.015f) {
            float w = 1.0f - u - v;
            if (w >= 0.0f && w <= 1.0f) {
                Vector3<float> P = A * u + B * v + C * w;
                pointCloud.push_back(P.x); pointCloud.push_back(P.y); pointCloud.push_back(P.z);
                pointCloud.push_back(u);   pointCloud.push_back(v);   pointCloud.push_back(w);
                pointCloud.push_back(2.0f); // size
                pointCloud.push_back(0.0f); // isLine
            }
        }
    }

    GLuint VAO_cloud, VBO_cloud;
    glGenVertexArrays(1, &VAO_cloud);
    glGenBuffers(1, &VBO_cloud);
    glBindVertexArray(VAO_cloud);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_cloud);
    glBufferData(GL_ARRAY_BUFFER, pointCloud.size() * sizeof(float), pointCloud.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    GLuint VAO_dyn, VBO_dyn;
    glGenVertexArrays(1, &VAO_dyn);
    glGenBuffers(1, &VBO_dyn);
    glBindVertexArray(VAO_dyn);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_dyn);
    glBufferData(GL_ARRAY_BUFFER, 1000 * 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.02f, 0.02f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Draw the point cloud (the iterated points)
        glBindVertexArray(VAO_cloud);
        glDrawArrays(GL_POINTS, 0, pointCloud.size() / 8);

        // Animate the white dot to simulate dragging
        float t = glfwGetTime();
        float u = (std::sin(t * 1.5f) * 0.5f + 0.5f);
        float v = (std::cos(t * 1.1f) * 0.5f + 0.5f);
        
        if (u + v > 1.0f) {
            u = 1.0f - u;
            v = 1.0f - v;
        }
        float w = 1.0f - u - v;
        
        if (w > 1.0f) {
            float diff = w - 1.0f;
            w = 1.0f;
            u += diff / 2.0f;
            v += diff / 2.0f;
        } else if (w < 0.0f) {
            float diff = -w;
            w = 0.0f;
            u -= diff / 2.0f;
            v -= diff / 2.0f;
        }

        Vector3<float> P = A * u + B * v + C * w;

        float sizeA = 10.0f + u * 30.0f;
        float sizeB = 10.0f + v * 30.0f;
        float sizeC = 10.0f + w * 30.0f;
        float sizeP = 12.0f;

        std::vector<float> dynData = {
            // Lines from P to A, B, C
            P.x, P.y, P.z, u, v, w, 1.0f, 1.0f,
            A.x, A.y, A.z, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            
            P.x, P.y, P.z, u, v, w, 1.0f, 1.0f,
            B.x, B.y, B.z, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            
            P.x, P.y, P.z, u, v, w, 1.0f, 1.0f,
            C.x, C.y, C.z, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

            // The animated dot itself (White)
            P.x, P.y, P.z, 1.0f, 1.0f, 1.0f, sizeP, 0.0f,

            // The 3 corner dots
            A.x, A.y, A.z, 1.0f, 0.0f, 0.0f, sizeA, 0.0f,
            B.x, B.y, B.z, 0.0f, 1.0f, 0.0f, sizeB, 0.0f,
            C.x, C.y, C.z, 0.0f, 0.0f, 1.0f, sizeC, 0.0f
        };

        char bufA[16], bufB[16], bufC[16];
        snprintf(bufA, sizeof(bufA), "%.2f", u);
        snprintf(bufB, sizeof(bufB), "%.2f", v);
        snprintf(bufC, sizeof(bufC), "%.2f", w);

        // Keep text at a fixed distance from the dots
        float distA = 0.13f;
        float distB = 0.13f;
        float distC = 0.13f;

        drawText(bufA, A.x - 0.05f, A.y + distA, 0.1f, Vector3<float>(1,0,0), dynData);
        drawText(bufB, B.x - 0.15f - distB, B.y - 0.05f - distB, 0.1f, Vector3<float>(0,1,0), dynData);
        drawText(bufC, C.x + 0.05f + distC, C.y - 0.05f - distC, 0.1f, Vector3<float>(0,0,1), dynData);

        glBindVertexArray(VAO_dyn);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_dyn);
        glBufferSubData(GL_ARRAY_BUFFER, 0, dynData.size() * sizeof(float), dynData.data());

        // Draw lines
        glDrawArrays(GL_LINES, 0, 6);
        
        // Draw text lines
        int numTextVertices = (dynData.size() / 8) - 10;
        if (numTextVertices > 0) {
            glDrawArrays(GL_LINES, 10, numTextVertices);
        }
        
        // Draw dots
        glDrawArrays(GL_POINTS, 6, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
