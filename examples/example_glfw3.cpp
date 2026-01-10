// main.cpp
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION

#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

// --- OpenGL / windowing ---
#include <GL/glew.h> // GLEW must come before GLFW
#include <GLFW/glfw3.h>
#include <tiny_gltf.h>

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

// Your 4x4 matrix type
using Mat4x4 = omath::opengl_engine::Mat4X4;

// Rotation angles for the Mesh
using RotationAngles = omath::opengl_engine::ViewAngles;

// Vertex: pos/normal = Vector3<float>, uv = Vector2<float>
using VertexType = omath::primitives::Vertex<Vector3<float>, omath::Vector2<float>>;
using MeshType = omath::opengl_engine::Mesh;
using MyCamera = omath::opengl_engine::Camera;
using Idx = Vector3<std::uint32_t>;

// ---------------- SHADERS (TEXTURED) ----------------

static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;

uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 vNormal;
out vec2 vUv;

void main() {
    // world-space normal (assuming no non-uniform scale)
    vNormal = mat3(uModel) * aNormal;
    vUv = aUv;
    gl_Position = uMVP * uModel * vec4(aPos, 1.0);
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vNormal;
in vec2 vUv;

uniform sampler2D uTexture;

out vec4 FragColor;

void main() {
    vec3 baseColor = texture(uTexture, vUv).rgb;

    // simple directional light
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(0.3, 0.6, 0.7));
    float diff = max(dot(N, L), 0.2); // some ambient floor

    FragColor = vec4(baseColor * diff, 1.0);
}
)";

// ---------------- GL helpers ----------------

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

// ---------------- tinygltf helpers ----------------

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

// ---------------- Node world transform (translation + scale) ----------------

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

    // ----- local translation -----
    Vector3<float> localTrans{0.f, 0.f, 0.f};

    // ----- local scale -----
    Vector3<float> localScale{1.f, 1.f, 1.f};

    if (node.matrix.size() == 16)
    {
        // glTF matrix is column-major
        const auto& m = node.matrix;

        // translation from last column
        localTrans.x = static_cast<float>(m[12]);
        localTrans.y = static_cast<float>(m[13]);
        localTrans.z = static_cast<float>(m[14]);

        // approximate scale = length of basis vectors
        float sx = vec3_length(static_cast<float>(m[0]), static_cast<float>(m[1]), static_cast<float>(m[2]));
        float sy = vec3_length(static_cast<float>(m[4]), static_cast<float>(m[5]), static_cast<float>(m[6]));
        float sz = vec3_length(static_cast<float>(m[8]), static_cast<float>(m[9]), static_cast<float>(m[10]));

        if (sx > 0.f)
            localScale.x = sx;
        if (sy > 0.f)
            localScale.y = sy;
        if (sz > 0.f)
            localScale.z = sz;
    }

    // node.translation overrides matrix translation if present
    if (node.translation.size() == 3)
    {
        localTrans.x = static_cast<float>(node.translation[0]);
        localTrans.y = static_cast<float>(node.translation[1]);
        localTrans.z = static_cast<float>(node.translation[2]);
    }

    // node.scale overrides matrix scale if present
    if (node.scale.size() == 3)
    {
        localScale.x = static_cast<float>(node.scale[0]);
        localScale.y = static_cast<float>(node.scale[1]);
        localScale.z = static_cast<float>(node.scale[2]);
    }

    // ----- accumulate to world -----
    Vector3<float> worldScale{parentScale.x * localScale.x, parentScale.y * localScale.y, parentScale.z * localScale.z};

    // (ignoring scale influence on translation; good enough for simple setups)
    Vector3<float> worldTrans{parentTrans.x + localTrans.x, parentTrans.y + localTrans.y, parentTrans.z + localTrans.z};

    outWorld[nodeIndex] = NodeWorldTransform{worldTrans, worldScale};

    for (int childIdx : node.children)
    {
        compute_node_world_transform_recursive(model, childIdx, worldTrans, worldScale, outWorld);
    }
}

static std::vector<NodeWorldTransform> compute_all_node_world_transforms(const tinygltf::Model& model)
{
    std::vector<NodeWorldTransform> world(
            model.nodes.size(), NodeWorldTransform{Vector3<float>{0.f, 0.f, 0.f}, Vector3<float>{1.f, 1.f, 1.f}});

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
            compute_node_world_transform_recursive(model, rootNodeIdx,
                                                   Vector3<float>{0.f, 0.f, 0.f}, // parent translation
                                                   Vector3<float>{1.f, 1.f, 1.f}, // parent scale
                                                   world);
        }
    }
    else
    {
        // No scenes defined: treat all nodes as roots
        for (size_t i = 0; i < model.nodes.size(); ++i)
        {
            compute_node_world_transform_recursive(model, static_cast<int>(i), Vector3<float>{0.f, 0.f, 0.f},
                                                   Vector3<float>{1.f, 1.f, 1.f}, world);
        }
    }

    return world;
}

// ---------------- Load meshes/primitives per node (origin + scale) ----------------

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

    // Precompute world translation + scale for all nodes
    std::vector<NodeWorldTransform> nodeWorld = compute_all_node_world_transforms(model);

    int primitiveIndexGlobal = 0;

    // Iterate over ALL nodes that reference a mesh
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

            // POSITION (required)
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

            // NORMAL (optional)
            const unsigned char* nrmBase = nullptr;
            size_t nrmStride = 0;
            auto nrmIt = prim.attributes.find("NORMAL");
            if (nrmIt != prim.attributes.end())
            {
                const tinygltf::Accessor& nrmAccessor = model.accessors[nrmIt->second];
                if (nrmAccessor.type == TINYGLTF_TYPE_VEC3
                    && nrmAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    nrmBase = get_accessor_data_ptr(model, nrmAccessor);
                    nrmStride = get_accessor_stride(model, nrmAccessor);
                }
            }

            // TEXCOORD_0 (optional, vec2)
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

            // Fill VBO
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

            // Build triangle EBO (Vector3<uint32_t>)
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
                    {
                        ebo.emplace_back(Idx{i, i + 1, i + 2});
                    }
                }
            }

            if (vbo.empty() || ebo.empty())
            {
                std::cerr << "Primitive produced empty vbo/ebo, skipping\n";
                continue;
            }

            // ---- Decide which texture index to use for this primitive ----
            int textureIndex = -1;

            // 1) Try material → baseColorTexture.index
            if (prim.material >= 0 && prim.material < static_cast<int>(model.materials.size()))
            {
                const tinygltf::Material& mat = model.materials[prim.material];
                if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
                {
                    textureIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
                }
            }

            // 2) If that failed but there are textures, map primitive index to textures round-robin
            if (textureIndex < 0 && !model.textures.empty())
            {
                textureIndex = primitiveIndexGlobal % static_cast<int>(model.textures.size());
            }

            outTextureIndices.push_back(textureIndex);

            // Create MeshType and store it, with origin & scale from node transform
            MeshType mesh{std::move(vbo), std::move(ebo)};
            mesh.set_origin(nodeOrigin); // origin from glTF node
            mesh.set_scale(nodeScale); // scale from glTF node
            mesh.set_rotation(RotationAngles{}); // keep your rotation system

            outMeshes.emplace_back(std::move(mesh));

            ++primitiveIndexGlobal;
        }
    }

    if (outMeshes.empty())
        throw std::runtime_error("No primitives with triangles were loaded from GLB: " + filename);

    outModel = std::move(model);
}

// ---------------- Texture creation from glTF ----------------

static GLuint create_default_white_texture()
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    unsigned char white[4] = {(std::uint8_t)(rand() % 255), (std::uint8_t)(rand() % 255), (std::uint8_t)(rand() % 255),
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

// textureIndex is an index into model.textures
static GLuint create_texture_from_gltf(const tinygltf::Model& model, int textureIndex)
{
    const tinygltf::Image* image = nullptr;

    if (textureIndex >= 0 && textureIndex < static_cast<int>(model.textures.size()))
    {
        const tinygltf::Texture& tex = model.textures[textureIndex];
        int imageIndex = tex.source;
        if (imageIndex >= 0 && imageIndex < static_cast<int>(model.images.size()))
        {
            image = &model.images[imageIndex];
        }
    }

    // Fallback: if textureIndex invalid or texture had no image, use first image if available
    if (!image && !model.images.empty())
    {
        image = &model.images[0];
    }

    if (!image)
        return create_default_white_texture();

    return create_texture_from_image(*image);
}

// ---------------- MAIN ----------------

int main(int argc, char** argv)
{
    // filename from CLI or default
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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "omath glTF multi-mesh (textured)", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSwapInterval(0);
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

    // ---------- GL state ----------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // ---------- Load GLB meshes (CPU side) ----------
    std::vector<MeshType> meshes;
    std::vector<int> textureIndices; // per-primitive texture index
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

    for (const auto& mesh : meshes)
    {
        colliders.emplace_back(mesh);
    }
    using ColliderInterface = omath::collision::ColliderInterface<omath::Vector3<float>>;

    for (size_t i = 0; i < meshCount; ++i)
    {
        MeshType& mesh = meshes[i];

        glBindVertexArray(vaos[i]);

        // VBO
        glBindBuffer(GL_ARRAY_BUFFER, vbos[i]);
        glBufferData(GL_ARRAY_BUFFER, mesh.m_vertex_buffer.size() * sizeof(VertexType), mesh.m_vertex_buffer.data(),
                     GL_STATIC_DRAW);

        // Flatten triangle EBO (Vector3<uint32_t>) to scalar index buffer
        flatIndices[i].reserve(mesh.m_element_buffer_object.size() * 3);
        for (const auto& tri : mesh.m_element_buffer_object)
        {
            flatIndices[i].push_back(tri.x);
            flatIndices[i].push_back(tri.y);
            flatIndices[i].push_back(tri.z);
        }

        // EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices[i].size() * sizeof(GLuint), flatIndices[i].data(),
                     GL_STATIC_DRAW);

        // vertex layout: position / normal / uv
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexType), (void*)offsetof(VertexType, uv));

        glBindVertexArray(0);

        // Texture for this mesh (based on its texture index)
        textures[i] = create_texture_from_gltf(gltfModel, textureIndices[i]);
    }

    // ---------- Camera setup ----------
    omath::projection::ViewPort viewPort{static_cast<float>(SCR_WIDTH), static_cast<float>(SCR_HEIGHT)};

    Vector3<float> camPos{0.f, 1.0f, 6.f};

    float nearPlane = 0.1f;
    float farPlane = 1000.f;
    auto fov = omath::projection::FieldOfView::from_degrees(110.f);

    MyCamera camera{camPos, {}, viewPort, fov, nearPlane, farPlane};

    // ---------- Shader ----------
    GLuint shaderProgram = createShaderProgram();
    GLint uMvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    GLint uModelLoc = glGetUniformLocation(shaderProgram, "uModel");
    GLint uTexLoc = glGetUniformLocation(shaderProgram, "uTexture");

    static float old_frame_time = glfwGetTime();

    auto cam_collider = colliders.at(0);
    // ---------- Main loop ----------
    // without fallback memory allocation on heap

    static std::array<std::byte, 1024*8> buf;
    std::pmr::monotonic_buffer_resource pool_stack{buf.data(), buf.size(),
                                             std::pmr::null_memory_resource()};

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - old_frame_time;
        old_frame_time = currentTime;
        glfwPollEvents();

        auto y = camera.get_origin().y;
        float speed = 40.f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.set_origin(camera.get_origin()
                              + omath::opengl_engine::forward_vector(camera.get_view_angles()) * speed * deltaTime);
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.set_origin(camera.get_origin()
                              - omath::opengl_engine::forward_vector(camera.get_view_angles()) * speed * deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.set_origin(camera.get_origin()
                              + omath::opengl_engine::right_vector(camera.get_view_angles()) * speed * deltaTime);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera.set_origin(camera.get_origin()
                              - omath::opengl_engine::right_vector(camera.get_view_angles()) * speed * deltaTime);
        }
        auto new_origin = camera.get_origin();
        new_origin.y = y;

        camera.set_origin(new_origin);
        float look_speed = 60;
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

        bool on_ground = false;

        for (int b = 0; b < colliders.size(); b++)
        {
            auto& collider_a = cam_collider;
            auto& collider_b = colliders.at(b);

            if (&collider_a == &collider_b)
                continue;
            auto info = omath::collision::GjkAlgorithm<ColliderInterface>::is_collide_with_simplex_info(collider_a,
                                                                                                        collider_b);
            if (!info.hit)
                continue;

            pool_stack.release();
            auto result = omath::collision::Epa<ColliderInterface>::solve(collider_a, collider_b, info.simplex, {}, pool_stack);

            const auto deg = result->penetration_vector.angle_between(omath::opengl_engine::k_abs_up)->as_degrees();
            on_ground |= deg > 150.f;

            //DO NOT PUSH OBJECT AWAY, NEED TO KEEP IT INSIDE OTHER OBJECTS TO
            //CHECK IF PLAYER STANDS ON SOMETHING
            if (std::abs(result->penetration_vector.y) <= 0.15 && deg > 150)
                continue;
            collider_a.set_origin(collider_a.get_origin() - result->penetration_vector * 1.005);
            camera.set_origin(camera.get_origin() - result->penetration_vector * 1.005);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && on_ground)
        {
            cam_collider.set_origin(cam_collider.get_origin() + omath::opengl_engine::k_abs_up * 5);
            camera.set_origin(cam_collider.get_origin() + omath::opengl_engine::k_abs_up * 5);
            on_ground = false;
        }

        if (!on_ground)
        {
            cam_collider.set_origin(cam_collider.get_origin() - omath::opengl_engine::k_abs_up * 5 * deltaTime);
            camera.set_origin(cam_collider.get_origin() - omath::opengl_engine::k_abs_up * 5 * deltaTime);
        }
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);

        viewPort.m_width = static_cast<float>(fbW);
        viewPort.m_height = static_cast<float>(fbH);
        camera.set_view_port(viewPort);

        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const Mat4x4& viewProj = camera.get_view_projection_matrix();
        glUseProgram(shaderProgram);

        const float* mvpPtr = viewProj.raw_array().data();
        glUniformMatrix4fv(uMvpLoc, 1, GL_FALSE, mvpPtr);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(uTexLoc, 0);

        // Render all meshes
        for (size_t i = 0; i < meshCount; ++i)
        {
            MeshType& mesh = meshes[i];

            const Mat4x4 model = mesh.get_to_world_matrix();
            const float* modelPtr = model.raw_array().data();
            glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, modelPtr);

            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glBindVertexArray(vaos[i]);

            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(flatIndices[i].size()), GL_UNSIGNED_INT, nullptr);
        }

        if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
        {
            std::println("FPS: {}", (int)(1 / deltaTime));
        }
        glfwSwapBuffers(window);
    }

    // ---------- Cleanup ----------
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