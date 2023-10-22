#pragma once
#include "tga/tga.hpp"
#include "tga/tga_math.hpp"

#if defined(_MSC_VER)
#pragma warning(push, 0)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#else
// Disable Warnings for external libs
#endif

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "tinyobjloader/tiny_obj_loader.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#else
// Enable Warnings again
#endif

namespace tga
{
struct Vertex {
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec2 uv;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec3 tangent;

    bool operator==(const Vertex& other) const
    {
        return position == other.position && uv == other.uv && normal == other.normal;
    }
    static tga::VertexLayout layout();
};

struct TextureBundle {
    Texture texture;
    uint32_t width, height;
    operator Texture() { return texture; }
};

struct Obj {
    std::vector<tga::Vertex> vertexBuffer;
    std::vector<uint32_t> indexBuffer;
};

struct Image {
    uint32_t width, height;
    uint32_t components;
    std::vector<uint8_t> data;
};

struct HDRImage {
    uint32_t width, height;
    uint32_t components;
    std::vector<float> data;
};

tga::Shader loadShader(std::string const& filepath, tga::ShaderType shaderType, tga::Interface& tgai);

TextureBundle loadTexture(std::string const& filepath, tga::Format format, tga::SamplerMode samplerMode,
                          tga::AddressMode addressMode, tga::Interface& tgai,
                          bool doGammaCorrection = false);

TextureBundle loadTexture(std::string const& filepath, tga::Format format, tga::SamplerMode samplerMode,
                          tga::Interface& tgai, bool doGammaCorrection = false);

Image loadImage(std::string const& filepath);
HDRImage loadHDRImage(std::string const& filepath, bool doGammaCorrection = false);

void writeHDR(std::string const& filename, uint32_t width, uint32_t height, tga::Format format,
              std::vector<float> const& data);

void writePNG(std::string const& filename, uint32_t width, uint32_t height, tga::Format format,
              std::vector<uint8_t> const& data);

/**
 * @brief A function to get access to the memory of something as a uint8_t* as required for most tga::Info structs
 *
 * @return The memory address as uint8_t*
 */
template <typename T>
uint8_t* memoryAccess(T& value)
{
    return reinterpret_cast<uint8_t*>(std::addressof(value));
}

template <typename T>
uint8_t* memoryAccess(std::vector<T>& vector)
{
    return reinterpret_cast<uint8_t*>(vector.data());
}

}  // namespace tga

template <>
struct std::hash<tga::Vertex> {
    std::size_t operator()(tga::Vertex const& v) const
    {
        return ((std::hash<glm::vec3>()(v.position) ^ (std::hash<glm::vec3>()(v.normal) << 1)) >> 1) ^
               (std::hash<glm::vec2>()(v.uv) << 1);
    }
};

namespace tga
{
template <typename T>
Obj loadObj(T const& filepath)
{
    // Using tinyobjloader to get the data
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, std::string(filepath).c_str()))
        throw std::runtime_error(warn + err);

    // Convert the data into our representation
    std::vector<Vertex> preVertexBuffer;
    uint32_t pvb_count = 0;
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex& vertex = preVertexBuffer.emplace_back();
            vertex.position = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],
                               attrib.vertices[3 * index.vertex_index + 2]};
            if (index.normal_index != -1)
                vertex.normal = {attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]};
            if (index.texcoord_index != -1)
                vertex.uv = {attrib.texcoords[2 * index.texcoord_index + 0],
                             1.f - attrib.texcoords[2 * index.texcoord_index + 1]};
            pvb_count++;
        }
    }
    assert(pvb_count == preVertexBuffer.size());
    std::unordered_map<Vertex, uint32_t> foundVertices{};
    std::vector<Vertex> vertexBuffer;
    std::vector<uint32_t> indexBuffer;

    // Calculate Tangents
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
    for (size_t i = 0; i < pvb_count; i += 3) {
        const auto& p0 = preVertexBuffer[i + 0].position;
        const auto& p1 = preVertexBuffer[i + 1].position;
        const auto& p2 = preVertexBuffer[i + 2].position;

        const auto& uv0 = preVertexBuffer[i + 0].uv;
        const auto& uv1 = preVertexBuffer[i + 1].uv;
        const auto& uv2 = preVertexBuffer[i + 2].uv;

        auto deltaPos1 = p1 - p0;
        auto deltaPos2 = p2 - p0;

        auto deltaUV1 = uv1 - uv0;
        auto deltaUV2 = uv2 - uv0;

        float r = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

        preVertexBuffer[i + 0].tangent = tangent;
        preVertexBuffer[i + 1].tangent = tangent;
        preVertexBuffer[i + 2].tangent = tangent;
    }

    // Fill final vertex and index Buffer
    uint32_t vb_count = 0;
    for (const auto& vertex : preVertexBuffer) {
        if (!foundVertices.count(vertex)) {    // It's a new Vertex
            foundVertices[vertex] = vb_count;  // static_cast<uint32_t>(vertexBuffer.size());
            vertexBuffer.emplace_back(vertex);
            vb_count++;
        } else {  // Seen before, average the tangents
            auto& v = vertexBuffer[foundVertices[vertex]];
            v.tangent += vertex.tangent;
        }
        indexBuffer.emplace_back(foundVertices[vertex]);
    }

    for (auto& vertex : vertexBuffer) vertex.tangent = glm::normalize(vertex.tangent);

    return {vertexBuffer, indexBuffer};
}
}  // namespace tga