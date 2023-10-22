#pragma once
#include "tga.hpp"

// Hash Functions for tga data types
template <>
struct std::hash<tga::Shader> {
    std::size_t operator()(tga::Shader const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaShader)key));
    }
};
template <>
struct std::hash<tga::Buffer> {
    std::size_t operator()(tga::Buffer const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaBuffer)key));
    }
};
template <>
struct std::hash<tga::Texture> {
    std::size_t operator()(tga::Texture const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaTexture)key));
    }
};
template <>
struct std::hash<tga::Window> {
    std::size_t operator()(tga::Window const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaWindow)key));
    }
};
template <>
struct std::hash<tga::InputSet> {
    std::size_t operator()(tga::InputSet const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaInputSet)key));
    }
};
template <>
struct std::hash<tga::RenderPass> {
    std::size_t operator()(tga::RenderPass const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaRenderPass)key));
    }
};
template <>
struct std::hash<tga::CommandBuffer> {
    std::size_t operator()(tga::CommandBuffer const& key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaCommandBuffer)key));
    }
};
