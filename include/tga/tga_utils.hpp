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
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 uv;
        glm::vec3 normal;
        glm::vec3 tangent;
        
        bool operator==(const Vertex &other) const{
            return position == other.position && uv == other.uv && normal == other.normal;
        }
        static tga::VertexLayout layout();
    };

    struct TextureBundle
    {
        Texture texture;
        uint32_t width, height;
        operator Texture(){return texture;}
    };

    struct Obj
    {
        std::vector<tga::Vertex> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    };

    struct Image
    {
        uint32_t width, height;
        uint32_t components;
        std::vector<uint8_t> data;
    };

    struct HDRImage
    {
        uint32_t width, height;
        uint32_t components;
        std::vector<float> data;
    };


    tga::Shader loadShader(std::string const& filepath, tga::ShaderType shaderType, std::shared_ptr<tga::Interface> const& tgai);

    TextureBundle loadTexture(std::string const& filepath, tga::Format format, tga::SamplerMode samplerMode, std::shared_ptr<tga::Interface> const& tgai, bool doGammaCorrection = false);

    Image loadImage(std::string const& filepath);
    HDRImage loadHDRImage(std::string const& filepath,bool doGammaCorrection = false);

    Obj loadObj(std::string const& filepath);

    void writeHDR(std::string const& filename, uint32_t width, uint32_t height, tga::Format format, std::vector<float> const& data);

    void writePNG(std::string const& filename, uint32_t width, uint32_t height, tga::Format format, std::vector<uint8_t> const& data);


    /**
     * @brief A function to get access to the memory of something as a uint8_t* as required for most tga::Info structs
     * 
     * @return The memory address as uint8_t*
     */
    template<typename T>
    uint8_t* memoryAccess(T &value)
    {
        return (uint8_t*)&value;
    }

    template<typename T>
    uint8_t* memoryAccess(std::vector<T> &vector)
    {
        return (uint8_t*)vector.data();
    }

}

namespace std
{
    template<> struct hash<tga::Vertex>{
        size_t operator()(tga::Vertex const &v) const{
            return ((hash<glm::vec3>()(v.position)^
                    (hash<glm::vec3>()(v.normal)<<1))>>1)^
                    (hash<glm::vec2>()(v.uv)<<1);
        }
    };
}
