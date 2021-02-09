#pragma once
#include "tga/tga.hpp"
#include "tga/tga_math.hpp"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "tinyobjloader/tiny_obj_loader.h"


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

    struct Obj
    {
        std::vector<tga::Vertex> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    };


    tga::Shader loadShader(std::string const& filepath, tga::ShaderType shaderType, std::shared_ptr<tga::Interface> const& tgai);

    tga::Texture loadTexture(std::string const& filepath, tga::Format format, tga::SamplerMode samplerMode, std::shared_ptr<tga::Interface> const& tgai);

    Obj loadObj(std::string const& filepath);
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
