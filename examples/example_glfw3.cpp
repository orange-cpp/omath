// main.cpp  (textured + improved shadows + film grain + chromatic aberration)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION

#include <GL/glew.h> // GLEW must come before GLFW
#include <GLFW/glfw3.h>
#include <tiny_gltf.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory_resource>
#include <print>
#include <stdexcept>
#include <vector>

// --- your math / engine stuff ---
#include "omath/3d_primitives/mesh.hpp"
#include "omath/collision/epa_algorithm.hpp"
#include "omath/collision/gjk_algorithm.hpp"
#include "omath/collision/line_tracer.hpp"
#include "omath/collision/mesh_collider.hpp"
#include "omath/engines/opengl_engine/camera.hpp"
#include "omath/engines/opengl_engine/constants.hpp"
#include "omath/engines/opengl_engine/mesh.hpp"
#include "omath/linear_algebra/vector2.hpp"
#include "omath/linear_algebra/vector3.hpp"

using omath::Vector3;

// ---------------- TYPE ALIASES ----------------
using Mat4x4 = omath::opengl_engine::Mat4X4;
using RotationAngles = omath::opengl_engine::ViewAngles;

using VertexType = omath::primitives::Vertex<Vector3<float>, omath::Vector2<float>>;
using MeshType = omath::opengl_engine::Mesh;
using MyCamera = omath::opengl_engine::Camera;
using Idx = Vector3<std::uint32_t>;

// ===================== SHADERS =====================

// ---- Main pass: textured + shadows ----
static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;

uniform mat4 uMVP;        // viewProj
uniform mat4 uModel;
uniform mat4 uLightSpace; // light VP

out vec3 vNormal;
out vec2 vUv;
out vec4 vFragPosLightSpace;

void main() {
    vNormal = mat3(uModel) * aNormal;
    vUv = aUv;

    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPosLightSpace = uLightSpace * worldPos;

    gl_Position = uMVP * worldPos;
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vNormal;
in vec2 vUv;
in vec4 vFragPosLightSpace;

uniform sampler2D uTexture;
uniform sampler2DShadow uShadowMap; // hardware compare
uniform vec3 uLightDir;

out vec4 FragColor;

float ShadowPCF(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

    // outside map => lit
    if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 0.0;

    // slope-scaled bias
    float bias = max(0.0012 * (1.0 - dot(N, L)), 0.00035);

    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    float shadow = 0.0;

    // 5x5 PCF
    for (int x = -2; x <= 2; ++x)
    for (int y = -2; y <= 2; ++y)
    {
        vec2 uv = proj.xy + vec2(x, y) * texel;
        // sampler2DShadow: returns "lit amount" in [0..1], so shadow = 1 - lit
        float lit = texture(uShadowMap, vec3(uv, (proj.z - bias)));
        shadow += (1.0 - lit);
    }

    shadow /= 25.0;
    return shadow;
}

void main()
{
    vec3 baseColor = texture(uTexture, vUv).rgb;

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);

    float ambient = 0.20;
    float diff = max(dot(N, L), 0.0);

    float shadow = ShadowPCF(vFragPosLightSpace, N, L);

    vec3 lit = baseColor * (ambient + (1.0 - shadow) * diff);
    FragColor = vec4(lit, 1.0);
}
)";

// ---- Shadow pass: depth only ----
static const char* shadowVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uLightSpace;
uniform mat4 uModel;

void main() {
    gl_Position = uLightSpace * uModel * vec4(aPos, 1.0);
}
)";

static const char* shadowFragmentShaderSource = R"(
#version 330 core
void main() { }
)";

// ---- Post: fullscreen triangle ----
static const char* postVertexShaderSource = R"(
#version 330 core
out vec2 vUv;

const vec2 pos[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

void main() {
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
    vUv = pos[gl_VertexID] * 0.5 + 0.5;
}
)";

static const char* postFragmentShaderSource = R"(
#version 330 core
in vec2 vUv;

uniform sampler2D uScene;
uniform vec2 uResolution;
uniform float uTime;

// tweakables
uniform float uChromaticStrength; // 0.002..0.004
uniform float uGrainStrength;     // 0.03..0.06
uniform float uGrainSize;         // 1.5..2.5

out vec4 FragColor;

float hash21(vec2 p)
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

void main()
{
    vec2 uv = vUv;

    // Chromatic aberration (radial)
    vec2 center = vec2(0.5, 0.5);
    vec2 d = uv - center;
    float r2 = dot(d, d);
    vec2 shift = d * (uChromaticStrength * (0.5 + 2.0 * r2));

    float r = texture(uScene, uv + shift).r;
    float g = texture(uScene, uv).g;
    float b = texture(uScene, uv - shift).b;

    vec3 col = vec3(r, g, b);

    // Film grain
    vec2 px = uv * (uResolution / max(uGrainSize, 0.0001));
    float n = hash21(px + vec2(uTime * 60.0, uTime * 30.0));
    float grain = (n - 0.5) * 2.0; // [-1,1]
    col += grain * uGrainStrength;

    FragColor = vec4(col, 1.0);
}
)";

// ===================== GL HELPERS =====================
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

static GLuint linkProgram(GLuint vs, GLuint fs, const char* label)
{
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
        std::cerr << label << " link error: " << log << std::endl;
    }
    return prog;
}

GLuint createShaderProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint prog = linkProgram(vs, fs, "Main program");
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

GLuint createShadowProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, shadowVertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, shadowFragmentShaderSource);
    GLuint prog = linkProgram(vs, fs, "Shadow program");
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

GLuint createPostProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, postVertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, postFragmentShaderSource);
    GLuint prog = linkProgram(vs, fs, "Post program");
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int w, int h)
{
    glViewport(0, 0, w, h);
}

// ===================== TINYGLTF HELPERS =====================
static const unsigned char* get_accessor_data_ptr(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
{
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
    return buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
}

static size_t get_accessor_stride(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
{
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    size_t stride = accessor.ByteStride(bufferView);
    if (stride == 0)
    {
        stride = tinygltf::GetComponentSizeInBytes(accessor.componentType)
                 * tinygltf::GetNumComponentsInType(accessor.type);
    }
    return stride;
}

static uint32_t read_index(const unsigned char* data, int componentType)
{
    switch (componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        return static_cast<uint32_t>(*reinterpret_cast<const uint8_t*>(data));
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return static_cast<uint32_t>(*reinterpret_cast<const uint16_t*>(data));
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        return *reinterpret_cast<const uint32_t*>(data);
    default:
        throw std::runtime_error("Unsupported index component type");
    }
}

// ===================== NODE TRANSFORM (translation + scale) =====================
static float vec3_length(float x, float y, float z)
{
    return std::sqrt(x * x + y * y + z * z);
}

struct NodeWorldTransform
{
    Vector3<float> translation;
    Vector3<float> scale;
};

static void compute_node_world_transform_recursive(const tinygltf::Model& model, int nodeIndex,
                                                   const Vector3<float>& parentTrans, const Vector3<float>& parentScale,
                                                   std::vector<NodeWorldTransform>& outWorld)
{
    const tinygltf::Node& node = model.nodes[nodeIndex];

    Vector3<float> localTrans{0.f, 0.f, 0.f};
    Vector3<float> localScale{1.f, 1.f, 1.f};

    if (node.matrix.size() == 16)
    {
        const auto& m = node.matrix;

        localTrans.x = static_cast<float>(m[12]);
        localTrans.y = static_cast<float>(m[13]);
        localTrans.z = static_cast<float>(m[14]);

        float sx = vec3_length(static_cast<float>(m[0]), static_cast<float>(m[1]), static_cast<float>(m[2]));
        float sy = vec3_length(static_cast<float>(m[4]), static_cast<float>(m[5]), static_cast<float>(m[6]));
        float sz = vec3_length(static_cast<float>(m[8]), static_cast<float>(m[9]), static_cast<float>(m[10]));

        if (sx > 0.f) localScale.x = sx;
        if (sy > 0.f) localScale.y = sy;
        if (sz > 0.f) localScale.z = sz;
    }

    if (node.translation.size() == 3)
    {
        localTrans.x = static_cast<float>(node.translation[0]);
        localTrans.y = static_cast<float>(node.translation[1]);
        localTrans.z = static_cast<float>(node.translation[2]);
    }

    if (node.scale.size() == 3)
    {
        localScale.x = static_cast<float>(node.scale[0]);
        localScale.y = static_cast<float>(node.scale[1]);
        localScale.z = static_cast<float>(node.scale[2]);
    }

    Vector3<float> worldScale{parentScale.x * localScale.x, parentScale.y * localScale.y, parentScale.z * localScale.z};
    Vector3<float> worldTrans{parentTrans.x + localTrans.x, parentTrans.y + localTrans.y, parentTrans.z + localTrans.z};

    outWorld[nodeIndex] = NodeWorldTransform{worldTrans, worldScale};

    for (int childIdx : node.children)
        compute_node_world_transform_recursive(model, childIdx, worldTrans, worldScale, outWorld);
}

static std::vector<NodeWorldTransform> compute_all_node_world_transforms(const tinygltf::Model& model)
{
    std::vector<NodeWorldTransform> world(
        model.nodes.size(),
        NodeWorldTransform{Vector3<float>{0.f, 0.f, 0.f}, Vector3<float>{1.f, 1.f, 1.f}});

    if (model.nodes.empty())
        return world;

    int sceneIndex = 0;
    if (!model.scenes.empty())
    {
        if (model.defaultScene >= 0 && model.defaultScene < static_cast<int>(model.scenes.size()))
            sceneIndex = model.defaultScene;
        else
            sceneIndex = 0;
    }

    if (!model.scenes.empty())
    {
        const tinygltf::Scene& scene = model.scenes[sceneIndex];
        for (int rootNodeIdx : scene.nodes)
        {
            compute_node_world_transform_recursive(
                model, rootNodeIdx,
                Vector3<float>{0.f, 0.f, 0.f},
                Vector3<float>{1.f, 1.f, 1.f},
                world);
        }
    }
    else
    {
        for (size_t i = 0; i < model.nodes.size(); ++i)
        {
            compute_node_world_transform_recursive(
                model, static_cast<int>(i),
                Vector3<float>{0.f, 0.f, 0.f},
                Vector3<float>{1.f, 1.f, 1.f},
                world);
        }
    }

    return world;
}

// ===================== LOAD GLB MESHES =====================
static void load_glb_meshes(const std::string& filename, tinygltf::Model& outModel, std::vector<MeshType>& outMeshes,
                            std::vector<int>& outTextureIndices)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ok = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (!warn.empty())
        std::cerr << "tinygltf warning: " << warn << std::endl;
    if (!ok)
        throw std::runtime_error("Failed to load GLB \"" + filename + "\": " + err);

    if (model.meshes.empty())
        throw std::runtime_error("GLB has no meshes: " + filename);

    outMeshes.clear();
    outTextureIndices.clear();

    std::vector<NodeWorldTransform> nodeWorld = compute_all_node_world_transforms(model);
    int primitiveIndexGlobal = 0;

    for (size_t nodeIndex = 0; nodeIndex < model.nodes.size(); ++nodeIndex)
    {
        const tinygltf::Node& node = model.nodes[nodeIndex];
        std::println("{}", node.name);

        if (node.mesh < 0 || node.mesh >= static_cast<int>(model.meshes.size()))
            continue;

        const tinygltf::Mesh& gltfMesh = model.meshes[node.mesh];
        const NodeWorldTransform& nodeTf = nodeWorld[nodeIndex];
        const Vector3<float>& nodeOrigin = nodeTf.translation;
        const Vector3<float>& nodeScale = nodeTf.scale;

        for (const tinygltf::Primitive& prim : gltfMesh.primitives)
        {
            if (prim.mode != TINYGLTF_MODE_TRIANGLES)
            {
                std::cerr << "Skipping non-triangle primitive\n";
                continue;
            }

            auto posIt = prim.attributes.find("POSITION");
            if (posIt == prim.attributes.end())
            {
                std::cerr << "Primitive has no POSITION attribute, skipping\n";
                continue;
            }

            const tinygltf::Accessor& posAccessor = model.accessors[posIt->second];
            if (posAccessor.type != TINYGLTF_TYPE_VEC3 || posAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
            {
                std::cerr << "POSITION must be VEC3 float, skipping primitive\n";
                continue;
            }

            const unsigned char* posBase = get_accessor_data_ptr(model, posAccessor);
            size_t posStride = get_accessor_stride(model, posAccessor);
            size_t vertexCount = posAccessor.count;

            std::vector<VertexType> vbo(vertexCount);

            const unsigned char* nrmBase = nullptr;
            size_t nrmStride = 0;
            auto nrmIt = prim.attributes.find("NORMAL");
            if (nrmIt != prim.attributes.end())
            {
                const tinygltf::Accessor& nrmAccessor = model.accessors[nrmIt->second];
                if (nrmAccessor.type == TINYGLTF_TYPE_VEC3 && nrmAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    nrmBase = get_accessor_data_ptr(model, nrmAccessor);
                    nrmStride = get_accessor_stride(model, nrmAccessor);
                }
            }

            const unsigned char* uvBase = nullptr;
            size_t uvStride = 0;
            auto uvIt = prim.attributes.find("TEXCOORD_0");
            if (uvIt != prim.attributes.end())
            {
                const tinygltf::Accessor& uvAccessor = model.accessors[uvIt->second];
                if (uvAccessor.type == TINYGLTF_TYPE_VEC2 && uvAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    uvBase = get_accessor_data_ptr(model, uvAccessor);
                    uvStride = get_accessor_stride(model, uvAccessor);
                }
            }

            for (size_t i = 0; i < vertexCount; ++i)
            {
                VertexType v{};

                const float* pos = reinterpret_cast<const float*>(posBase + i * posStride);
                v.position = Vector3<float>{pos[0], pos[1], pos[2]};

                if (nrmBase)
                {
                    const float* nrm = reinterpret_cast<const float*>(nrmBase + i * nrmStride);
                    v.normal = Vector3<float>{nrm[0], nrm[1], nrm[2]};
                }
                else
                {
                    v.normal = Vector3<float>{0.f, 0.f, 1.f};
                }

                if (uvBase)
                {
                    const float* uv = reinterpret_cast<const float*>(uvBase + i * uvStride);
                    v.uv = omath::Vector2<float>{uv[0], uv[1]};
                }
                else
                {
                    v.uv = omath::Vector2<float>{0.f, 0.f};
                }

                vbo[i] = v;
            }

            std::vector<Idx> ebo;
            if (prim.indices >= 0)
            {
                const tinygltf::Accessor& idxAccessor = model.accessors[prim.indices];
                const unsigned char* idxBase = get_accessor_data_ptr(model, idxAccessor);
                size_t idxStride = get_accessor_stride(model, idxAccessor);
                size_t indexCount = idxAccessor.count;

                if (indexCount < 3)
                    continue;

                ebo.reserve(indexCount / 3);
                for (size_t i = 0; i + 2 < indexCount; i += 3)
                {
                    const unsigned char* p0 = idxBase + (i + 0) * idxStride;
                    const unsigned char* p1 = idxBase + (i + 1) * idxStride;
                    const unsigned char* p2 = idxBase + (i + 2) * idxStride;

                    uint32_t i0 = read_index(p0, idxAccessor.componentType);
                    uint32_t i1 = read_index(p1, idxAccessor.componentType);
                    uint32_t i2 = read_index(p2, idxAccessor.componentType);

                    ebo.emplace_back(Idx{i0, i1, i2});
                }
            }
            else
            {
                if (vertexCount >= 3)
                {
                    ebo.reserve(vertexCount / 3);
                    for (uint32_t i = 0; i + 2 < vertexCount; i += 3)
                        ebo.emplace_back(Idx{i, i + 1, i + 2});
                }
            }

            if (vbo.empty() || ebo.empty())
            {
                std::cerr << "Primitive produced empty vbo/ebo, skipping\n";
                continue;
            }

            int textureIndex = -1;
            if (prim.material >= 0 && prim.material < static_cast<int>(model.materials.size()))
            {
                const tinygltf::Material& mat = model.materials[prim.material];
                if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
                    textureIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
            }
            if (textureIndex < 0 && !model.textures.empty())
                textureIndex = primitiveIndexGlobal % static_cast<int>(model.textures.size());

            outTextureIndices.push_back(textureIndex);

            MeshType mesh{std::move(vbo), std::move(ebo)};
            mesh.set_origin(nodeOrigin);
            mesh.set_scale(nodeScale);
            mesh.set_rotation(RotationAngles{});

            outMeshes.emplace_back(std::move(mesh));
            ++primitiveIndexGlobal;
        }
    }

    if (outMeshes.empty())
        throw std::runtime_error("No primitives with triangles were loaded from GLB: " + filename);

    outModel = std::move(model);
}

// ===================== TEXTURES =====================
static GLuint create_default_white_texture()
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    unsigned char white[4] = {
        (std::uint8_t)(rand() % 255),
        (std::uint8_t)(rand() % 255),
        (std::uint8_t)(rand() % 255),
        255};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return tex;
}

static GLuint create_texture_from_image(const tinygltf::Image& image)
{
    if (image.image.empty() || image.width <= 0 || image.height <= 0)
    {
        std::cerr << "Image is empty or invalid, using white texture\n";
        return create_default_white_texture();
    }

    GLenum format = GL_RGBA;
    if (image.component == 3)
        format = GL_RGB;
    else if (image.component == 4)
        format = GL_RGBA;
    else if (image.component == 1)
        format = GL_RED;

    GLuint glTex = 0;
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.image.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return glTex;
}

static GLuint create_texture_from_gltf(const tinygltf::Model& model, int textureIndex)
{
    const tinygltf::Image* image = nullptr;

    if (textureIndex >= 0 && textureIndex < static_cast<int>(model.textures.size()))
    {
        const tinygltf::Texture& tex = model.textures[textureIndex];
        int imageIndex = tex.source;
        if (imageIndex >= 0 && imageIndex < static_cast<int>(model.images.size()))
            image = &model.images[imageIndex];
    }

    if (!image && !model.images.empty())
        image = &model.images[0];

    if (!image)
        return create_default_white_texture();

    return create_texture_from_image(*image);
}

// ===================== MINIMAL MATH FOR LIGHT MATRICES =====================
struct Mat4f
{
    float m[16];
};

static Mat4f mat4_identity()
{
    Mat4f r{};
    r.m[0] = 1.f;
    r.m[5] = 1.f;
    r.m[10] = 1.f;
    r.m[15] = 1.f;
    return r;
}

static Mat4f mat4_mul(const Mat4f& a, const Mat4f& b)
{
    Mat4f r{};
    // column-major multiply: r = a*b
    for (int c = 0; c < 4; ++c)
    for (int rr = 0; rr < 4; ++rr)
    {
        r.m[c * 4 + rr] =
            a.m[0 * 4 + rr] * b.m[c * 4 + 0] +
            a.m[1 * 4 + rr] * b.m[c * 4 + 1] +
            a.m[2 * 4 + rr] * b.m[c * 4 + 2] +
            a.m[3 * 4 + rr] * b.m[c * 4 + 3];
    }
    return r;
}

static float v3_dot(Vector3<float> a, Vector3<float> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vector3<float> v3_cross(Vector3<float> a, Vector3<float> b)
{
    return Vector3<float>{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

static Vector3<float> v3_norm(Vector3<float> v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len <= 0.0f)
        return Vector3<float>{0, 0, 0};
    return Vector3<float>{v.x / len, v.y / len, v.z / len};
}

static Mat4f mat4_ortho(float l, float r, float b, float t, float n, float f)
{
    Mat4f m{};
    m.m[0] = 2.f / (r - l);
    m.m[5] = 2.f / (t - b);
    m.m[10] = -2.f / (f - n);
    m.m[12] = -(r + l) / (r - l);
    m.m[13] = -(t + b) / (t - b);
    m.m[14] = -(f + n) / (f - n);
    m.m[15] = 1.f;
    return m;
}

static Mat4f mat4_lookAt(Vector3<float> eye, Vector3<float> center, Vector3<float> up)
{
    Vector3<float> f = v3_norm(Vector3<float>{center.x - eye.x, center.y - eye.y, center.z - eye.z});
    Vector3<float> s = v3_norm(v3_cross(f, up));
    Vector3<float> u = v3_cross(s, f);

    Mat4f m = mat4_identity();
    m.m[0] = s.x;
    m.m[4] = s.y;
    m.m[8] = s.z;

    m.m[1] = u.x;
    m.m[5] = u.y;
    m.m[9] = u.z;

    m.m[2] = -f.x;
    m.m[6] = -f.y;
    m.m[10] = -f.z;

    m.m[12] = -v3_dot(s, eye);
    m.m[13] = -v3_dot(u, eye);
    m.m[14] = v3_dot(f, eye);
    return m;
}

// ===================== MAIN =====================
int main(int argc, char** argv)
{
    std::string glbFile = (argc > 1) ? argv[1] : "untitled.glb";

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

    constexpr int SCR_WIDTH = 800;
    constexpr int SCR_HEIGHT = 600;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "omath glTF (shadows + post)", nullptr, nullptr);
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
        std::cerr << "Failed to initialize GLEW: " << reinterpret_cast<const char*>(glewGetErrorString(glewErr)) << "\n";
        glfwTerminate();
        return -1;
    }

    // ---------- GL state ----------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // ---------- Load GLB meshes ----------
    std::vector<MeshType> meshes;
    std::vector<int> textureIndices;
    tinygltf::Model gltfModel;

    try
    {
        load_glb_meshes(glbFile, gltfModel, meshes, textureIndices);
        std::cerr << "Loaded " << meshes.size() << " mesh primitives from GLB\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error loading GLB: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    const size_t meshCount = meshes.size();

    // ---------- Create GL buffers per mesh ----------
    std::vector<GLuint> vaos(meshCount);
    std::vector<GLuint> vbos(meshCount);
    std::vector<GLuint> ebos(meshCount);
    std::vector<GLuint> textures(meshCount);
    std::vector<std::vector<GLuint>> flatIndices(meshCount);

    glGenVertexArrays(static_cast<GLsizei>(meshCount), vaos.data());
    glGenBuffers(static_cast<GLsizei>(meshCount), vbos.data());
    glGenBuffers(static_cast<GLsizei>(meshCount), ebos.data());

    using Collider = omath::collision::MeshCollider<MeshType>;
    std::vector<Collider> colliders;
    colliders.reserve(meshCount);
    for (const auto& mesh : meshes)
        colliders.emplace_back(mesh);

    using ColliderInterface = omath::collision::ColliderInterface<omath::Vector3<float>>;

    for (size_t i = 0; i < meshCount; ++i)
    {
        MeshType& mesh = meshes[i];

        glBindVertexArray(vaos[i]);

        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.m_vertex_buffer.size() * sizeof(VertexType),
                     mesh.m_vertex_buffer.data(),
                     GL_STATIC_DRAW);

        flatIndices[i].reserve(mesh.m_element_buffer_object.size() * 3);
        for (const auto& tri : mesh.m_element_buffer_object)
        {
            flatIndices[i].push_back(tri.x);
            flatIndices[i].push_back(tri.y);
            flatIndices[i].push_back(tri.z);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     flatIndices[i].size() * sizeof(GLuint),
                     flatIndices[i].data(),
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, uv));

        glBindVertexArray(0);

        textures[i] = create_texture_from_gltf(gltfModel, textureIndices[i]);
    }

    // ---------- Shadow map FBO (improved quality) ----------
    constexpr int SHADOW_W = 4096;
    constexpr int SHADOW_H = 4096;

    GLuint shadowFBO = 0;
    GLuint shadowDepthTex = 0;

    glGenFramebuffers(1, &shadowFBO);
    glGenTextures(1, &shadowDepthTex);

    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = {1.f, 1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    // hardware depth compare for sampler2DShadow
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Shadow framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------- Scene FBO for post-processing (color + depth) ----------
    GLuint sceneFBO = 0;
    GLuint sceneColorTex = 0;
    GLuint sceneDepthRBO = 0;

    glGenFramebuffers(1, &sceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    glGenTextures(1, &sceneColorTex);
    glBindTexture(GL_TEXTURE_2D, sceneColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTex, 0);

    glGenRenderbuffers(1, &sceneDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, sceneDepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Scene framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---------- Camera setup ----------
    omath::projection::ViewPort viewPort{static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT)};
    Vector3<float> camPos{0.f, 1.0f, 6.f};

    float nearPlane = 0.1f;
    float farPlane = 1000.f;
    auto fov = omath::projection::FieldOfView::from_degrees(110.f);

    MyCamera camera{camPos, {}, viewPort, fov, nearPlane, farPlane};

    // ---------- Programs ----------
    GLuint shaderProgram = createShaderProgram();
    GLint uMvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    GLint uModelLoc = glGetUniformLocation(shaderProgram, "uModel");
    GLint uTexLoc = glGetUniformLocation(shaderProgram, "uTexture");
    GLint uLightSpaceLoc = glGetUniformLocation(shaderProgram, "uLightSpace");
    GLint uShadowMapLoc = glGetUniformLocation(shaderProgram, "uShadowMap");
    GLint uLightDirLoc = glGetUniformLocation(shaderProgram, "uLightDir");

    GLuint shadowProgram = createShadowProgram();
    GLint sh_uLightSpace = glGetUniformLocation(shadowProgram, "uLightSpace");
    GLint sh_uModel = glGetUniformLocation(shadowProgram, "uModel");

    GLuint postProgram = createPostProgram();
    GLint postSceneLoc = glGetUniformLocation(postProgram, "uScene");
    GLint postResLoc = glGetUniformLocation(postProgram, "uResolution");
    GLint postTimeLoc = glGetUniformLocation(postProgram, "uTime");
    GLint postChromLoc = glGetUniformLocation(postProgram, "uChromaticStrength");
    GLint postGrainLoc = glGetUniformLocation(postProgram, "uGrainStrength");
    GLint postSizeLoc = glGetUniformLocation(postProgram, "uGrainSize");

    // core profile needs a VAO bound for glDrawArrays, even with gl_VertexID
    GLuint postVAO = 0;
    glGenVertexArrays(1, &postVAO);

    static float old_frame_time = (float)glfwGetTime();
    auto cam_collider = colliders.at(0);

    // without fallback memory allocation on heap
    static std::array<std::byte, 1024 * 8> buf;
    std::pmr::monotonic_buffer_resource pool_stack{buf.data(), buf.size(), std::pmr::null_memory_resource()};

    static int lastW = SCR_WIDTH;
    static int lastH = SCR_HEIGHT;

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - old_frame_time;
        old_frame_time = currentTime;

        glfwPollEvents();

        // ---------- Movement ----------
        auto y = camera.get_origin().y;
        float speed = 40.f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.set_origin(camera.get_origin()
                              + omath::opengl_engine::forward_vector(camera.get_view_angles()) * speed * deltaTime);

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.set_origin(camera.get_origin()
                              - omath::opengl_engine::forward_vector(camera.get_view_angles()) * speed * deltaTime);

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.set_origin(camera.get_origin()
                              + omath::opengl_engine::right_vector(camera.get_view_angles()) * speed * deltaTime);

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.set_origin(camera.get_origin()
                              - omath::opengl_engine::right_vector(camera.get_view_angles()) * speed * deltaTime);

        auto new_origin = camera.get_origin();
        new_origin.y = y;
        camera.set_origin(new_origin);

        float look_speed = 60.f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            auto view_angles = camera.get_view_angles();
            view_angles.pitch += omath::opengl_engine::PitchAngle::from_degrees(look_speed * deltaTime);
            camera.set_view_angles(view_angles);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            auto view_angles = camera.get_view_angles();
            view_angles.pitch -= omath::opengl_engine::PitchAngle::from_degrees(look_speed * deltaTime);
            camera.set_view_angles(view_angles);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            auto view_angles = camera.get_view_angles();
            view_angles.yaw += omath::opengl_engine::YawAngle::from_degrees(look_speed * deltaTime);
            camera.set_view_angles(view_angles);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            auto view_angles = camera.get_view_angles();
            view_angles.yaw -= omath::opengl_engine::YawAngle::from_degrees(look_speed * deltaTime);
            camera.set_view_angles(view_angles);
        }

        cam_collider.set_origin(camera.get_origin());

        // ---------- Collision ----------
        bool on_ground = false;

        for (int b = 0; b < (int)colliders.size(); b++)
        {
            auto& collider_a = cam_collider;
            auto& collider_b = colliders.at(b);

            if (&collider_a == &collider_b)
                continue;

            auto info = omath::collision::GjkAlgorithm<ColliderInterface>::is_collide_with_simplex_info(collider_a, collider_b);
            if (!info.hit)
                continue;

            pool_stack.release();
            auto result = omath::collision::Epa<ColliderInterface>::solve(collider_a, collider_b, info.simplex, {}, pool_stack);

            const auto deg = result->penetration_vector.angle_between(omath::opengl_engine::k_abs_up)->as_degrees();
            on_ground |= deg > 150.f;

            if (std::abs(result->penetration_vector.y) <= 0.15f && deg > 150.f)
                continue;

            collider_a.set_origin(collider_a.get_origin() - result->penetration_vector * 1.005f);
            camera.set_origin(camera.get_origin() - result->penetration_vector * 1.005f);
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && on_ground)
        {
            cam_collider.set_origin(cam_collider.get_origin() + omath::opengl_engine::k_abs_up * 5.f);
            camera.set_origin(cam_collider.get_origin() + omath::opengl_engine::k_abs_up * 5.f);
            on_ground = false;
        }

        if (!on_ground)
        {
            cam_collider.set_origin(cam_collider.get_origin() - omath::opengl_engine::k_abs_up * 5.f * deltaTime);
            camera.set_origin(cam_collider.get_origin() - omath::opengl_engine::k_abs_up * 5.f * deltaTime);
        }

        // ---------- Framebuffer size / resize scene FBO ----------
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW <= 0 || fbH <= 0)
        {
            // window minimized; skip rendering this frame
            glfwSwapBuffers(window);
            continue;
        }

        if (fbW != lastW || fbH != lastH)
        {
            lastW = fbW;
            lastH = fbH;

            glBindTexture(GL_TEXTURE_2D, sceneColorTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbW, fbH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glBindRenderbuffer(GL_RENDERBUFFER, sceneDepthRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbW, fbH);
        }

        viewPort.m_width = static_cast<float>(fbW);
        viewPort.m_height = static_cast<float>(fbH);
        camera.set_view_port(viewPort);

        // ---------- Build light-space matrix (sun) ----------
        Vector3<float> lightDir = v3_norm(Vector3<float>{0.3f, 0.6f, 0.7f}); // towards light
        Vector3<float> center = camera.get_origin();

        Vector3<float> lightPos{
            center.x - lightDir.x * 80.f,
            center.y - lightDir.y * 80.f,
            center.z - lightDir.z * 80.f};

        Mat4f lightProj = mat4_ortho(-80.f, 80.f, -80.f, 80.f, 1.f, 200.f);
        Mat4f lightView = mat4_lookAt(lightPos, center, Vector3<float>{0.f, 1.f, 0.f});
        Mat4f lightSpace = mat4_mul(lightProj, lightView);

        // ===================== 1) SHADOW PASS =====================
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glViewport(0, 0, SHADOW_W, SHADOW_H);
        glClear(GL_DEPTH_BUFFER_BIT);

        // big acne reducer
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        // optional (often helps further)
        glCullFace(GL_FRONT);

        glUseProgram(shadowProgram);
        glUniformMatrix4fv(sh_uLightSpace, 1, GL_FALSE, lightSpace.m);

        for (size_t i = 0; i < meshCount; ++i)
        {
            const Mat4x4 model = meshes[i].get_to_world_matrix();
            glUniformMatrix4fv(sh_uModel, 1, GL_FALSE, model.raw_array().data());

            glBindVertexArray(vaos[i]);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(flatIndices[i].size()), GL_UNSIGNED_INT, nullptr);
        }

        glCullFace(GL_BACK);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ===================== 2) SCENE PASS (to sceneFBO) =====================
        glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
        glViewport(0, 0, fbW, fbH);

        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const Mat4x4& viewProj = camera.get_view_projection_matrix();

        glUseProgram(shaderProgram);

        glUniformMatrix4fv(uMvpLoc, 1, GL_FALSE, viewProj.raw_array().data());
        glUniformMatrix4fv(uLightSpaceLoc, 1, GL_FALSE, lightSpace.m);
        glUniform3f(uLightDirLoc, lightDir.x, lightDir.y, lightDir.z);

        // shadow map on unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
        glUniform1i(uShadowMapLoc, 1);

        // base texture on unit 0
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(uTexLoc, 0);

        for (size_t i = 0; i < meshCount; ++i)
        {
            const Mat4x4 model = meshes[i].get_to_world_matrix();
            glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, model.raw_array().data());

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);

            glBindVertexArray(vaos[i]);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(flatIndices[i].size()), GL_UNSIGNED_INT, nullptr);
        }

        // ===================== 3) POST PASS (to screen) =====================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbW, fbH);

        glDisable(GL_DEPTH_TEST);

        glUseProgram(postProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneColorTex);
        glUniform1i(postSceneLoc, 0);

        glUniform2f(postResLoc, (float)fbW, (float)fbH);
        glUniform1f(postTimeLoc, currentTime);

        // tweak values here
        glUniform1f(postChromLoc, 0.010f);
        glUniform1f(postGrainLoc, 0.000f);
        glUniform1f(postSizeLoc, 1.8f);

        glBindVertexArray(postVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glEnable(GL_DEPTH_TEST);

        if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS && deltaTime > 0.0f)
            std::println("FPS: {}", (int)(1.f / deltaTime));

        glfwSwapBuffers(window);
    }

    // ---------- Cleanup ----------
    glDeleteVertexArrays(1, &postVAO);
    glDeleteProgram(postProgram);

    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteTextures(1, &sceneColorTex);
    glDeleteRenderbuffers(1, &sceneDepthRBO);

    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowDepthTex);
    glDeleteProgram(shadowProgram);

    for (size_t i = 0; i < meshCount; ++i)
    {
        glDeleteTextures(1, &textures[i]);
        glDeleteVertexArrays(1, &vaos[i]);
        glDeleteBuffers(1, &vbos[i]);
        glDeleteBuffers(1, &ebos[i]);
    }
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}