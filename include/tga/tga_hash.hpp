#pragma once
#include "tga.hpp"

// Hash Functions for tga data types
template <>
struct std::hash<tga::Shader> {
    std::size_t operator()(const tga::Shader &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaShader)key));
    }
};
template <>
struct std::hash<tga::Buffer> {
    std::size_t operator()(const tga::Buffer &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaBuffer)key));
    }
};
template <>
struct std::hash<tga::Texture> {
    std::size_t operator()(const tga::Texture &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaTexture)key));
    }
};
template <>
struct std::hash<tga::Window> {
    std::size_t operator()(const tga::Window &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaWindow)key));
    }
};
template <>
struct std::hash<tga::InputSet> {
    std::size_t operator()(const tga::InputSet &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaInputSet)key));
    }
};
template <>
struct std::hash<tga::RenderPass> {
    std::size_t operator()(const tga::RenderPass &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaRenderPass)key));
    }
};
template <>
struct std::hash<tga::CommandBuffer> {
    std::size_t operator()(const tga::CommandBuffer &key) const
    {
        return std::hash<uint64_t>()(reinterpret_cast<uint64_t>((TgaCommandBuffer)key));
    }
};
