#pragma once
#include "vulkan/vulkan.hpp"

namespace tga
{
    struct QueueIndices {
        uint32_t graphics;
        uint32_t transfer;
    };

    struct Shader_TV {
        vk::ShaderModule module;
        tga::ShaderType type;
    };
    struct Buffer_TV {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::BufferUsageFlags flags;
        vk::DeviceSize size;
    };

    struct Texture_TV {
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory memory;
        vk::Sampler sampler;
        vk::Extent3D extent;
        vk::Format format;
    };

    struct DepthBuffer_TV {
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory memory;
    };

    struct InputSet_TV {
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
        uint32_t index;
    };

    struct RenderPass_TV {
        std::vector<vk::Framebuffer> framebuffers;
        vk::RenderPass renderPass;
        std::vector<vk::DescriptorSetLayout> setLayouts;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline pipeline;
        vk::PipelineBindPoint bindPoint;
        vk::Extent2D area;
    };

    struct CommandBuffer_TV {
        vk::CommandBuffer cmdBuffer;
    };

}  // namespace tga