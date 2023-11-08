#pragma once
#include "tga/tga.hpp"
#include "vulkan/vulkan.hpp"

namespace tga
{
namespace vkData
{
    struct Shader {
        vk::ShaderModule module{};
        tga::ShaderType type;
    };
    struct Buffer {
        vk::Buffer buffer{};
        vk::DeviceMemory memory;
        vk::BufferUsageFlags flags;
        vk::DeviceSize size;
    };

    struct StagingBuffer {
        vk::Buffer buffer{};
        void *mapping;
        vk::DeviceMemory memory;
    };

    struct DepthBuffer {
        vk::Image image{};
        vk::ImageView imageView;
        vk::DeviceMemory memory;
    };

    struct Texture {
        vk::Image image{};
        vk::ImageView imageView;
        vk::DeviceMemory memory;
        vk::Sampler sampler;
        vk::Extent3D extent;
        vk::Format format;

        DepthBuffer depthBuffer;
    };

    struct InputSet {
        vk::DescriptorPool descriptorPool{};
        vk::DescriptorSet descriptorSet;
        vk::PipelineBindPoint pipelineBindPoint;
        vk::PipelineLayout pipelineLayout;
        uint32_t index;
    };

    struct Layout {
        vk::PipelineLayout pipelineLayout{};
        std::vector<vk::DescriptorSetLayout> setLayouts;
        std::vector<std::vector<vk::DescriptorType>> setDescriptorTypes;
    };

    struct RenderPass {
        vk::Pipeline pipeline{};
        vk::RenderPass renderPass;
        std::vector<vk::Framebuffer> framebuffers;
        size_t numColorAttachmentsPerFrameBuffer;
        vk::Extent2D area;
        Layout layout;
    };

    struct ComputePass {
        vk::Pipeline pipeline{};
        Layout layout;
    };

    struct CommandBuffer {
        vk::CommandBuffer cmdBuffer{};
        vk::Fence completionFence{};
        vk::RenderPass currentRenderPass{};
    };

    struct Window {
        vk::SurfaceKHR surface{};
        vk::SwapchainKHR swapchain;
        vk::Extent2D extent;
        vk::Format format;
        std::any nativeHandle;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        std::vector<vk::Semaphore> imageAcquiredSignals{};
        uint32_t nextAcquireSignal{0};
        std::vector<vk::Semaphore> renderCompletedSignals{};
        uint32_t nextRenderSignal{0};
        std::vector<vk::CommandBuffer> toColorAttachmentTransitionCmds{};
        std::vector<vk::CommandBuffer> toPresentSrcTransitionCmds{};
        DepthBuffer depthBuffer{};
    };

    namespace ext
    {
        struct AccelerationStructure {
            vk::AccelerationStructureKHR accelerationStructure{};
            vk::Buffer buffer;
            vk::DeviceMemory memory;
        };
    }  // namespace ext

}  // namespace vkData
}  // namespace tga