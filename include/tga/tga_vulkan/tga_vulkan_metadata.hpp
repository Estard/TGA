#pragma once
#include "tga/tga.hpp"
#include "vulkan/vulkan.hpp"

namespace tga
{
    struct QueueIndices {
        uint32_t graphics;
        uint32_t transfer;
    };

    struct Shader_vkData {
        vk::ShaderModule module;
        tga::ShaderType type;
    };
    struct Buffer_vkData {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
        vk::BufferUsageFlags flags;
        vk::DeviceSize size;
    };

    struct Texture_vkData {
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory memory;
        vk::Sampler sampler;
        vk::Extent3D extent;
        vk::Format format;
    };

    struct DepthBuffer_vkData {
        vk::Image image;
        vk::ImageView imageView;
        vk::DeviceMemory memory;
    };

    struct InputSet_vkData {
        vk::DescriptorPool descriptorPool;
        vk::DescriptorSet descriptorSet;
        uint32_t index;
    };

    struct RenderPass_vkData {
        std::vector<vk::Framebuffer> framebuffers;
        vk::RenderPass renderPass;
        std::vector<vk::DescriptorSetLayout> setLayouts;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline pipeline;
        vk::PipelineBindPoint bindPoint;
        vk::Extent2D area;
    };

    struct CommandBuffer_vkData {
        vk::CommandBuffer cmdBuffer;
    };

     struct Window_vkData {
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        vk::Extent2D extent;
        vk::Format format;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        std::any nativeHandle;
        vk::Fence inFlightFence;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
        uint32_t currentFrameIndex;
    };

}  // namespace tga