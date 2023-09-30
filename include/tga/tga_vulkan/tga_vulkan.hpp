#pragma once
#include "tga/tga.hpp"
#include "tga/tga_hash.hpp"
#include "tga_vulkan_WSI.hpp"
#include "tga_vulkan_metadata.hpp"

namespace tga
{
    /** \brief The Interface Implementation over the Vulkan API
     */
    struct Interface::InternalState{
        InternalState();
        ~InternalState();
        // Vulkan Stuff
        VulkanWSI wsi;
        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugger;
        vk::PhysicalDevice pDevice;
        uint32_t renderQueueFamiliy;
        vk::Device device;
        vk::Queue renderQueue;
        vk::CommandPool transferCmdPool;
        vk::CommandPool graphicsCmdPool;

        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
        Buffer_vkData allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                     vk::MemoryPropertyFlags properties);
        std::tuple<vk::ImageType, vk::ImageViewType, vk::ImageCreateFlags> determineImageTypeInfo(
            const TextureInfo &textureInfo);
        std::tuple<vk::Extent3D, uint32_t> determineImageDimensions(const TextureInfo &textureInfo);
        std::pair<vk::ImageTiling, vk::ImageUsageFlags> determineImageFeatures(vk::Format &format);
        vk::Format findDepthFormat();
        DepthBuffer_vkData createDepthBuffer(uint32_t width, uint32_t height);
        vk::RenderPass makeRenderPass(std::vector<vk::Format> const &colorFormats, ClearOperation clearOps,
                                      vk::ImageLayout layout);
        std::vector<vk::DescriptorSetLayout> decodeInputLayout(InputLayout const &inputLayout);
        vk::Pipeline makeGraphicsPipeline(const RenderPassInfo &renderPassInfo, vk::PipelineLayout pipelineLayout,
                                          vk::RenderPass renderPass);
        std::pair<vk::Pipeline, vk::PipelineBindPoint> makePipeline(const RenderPassInfo &renderPassInfo,
                                                                    vk::PipelineLayout pipelineLayout,
                                                                    vk::RenderPass renderPass);

        vk::CommandBuffer beginOneTimeCmdBuffer(vk::CommandPool &cmdPool);
        void endOneTimeCmdBuffer(vk::CommandBuffer &cmdBuffer, vk::CommandPool &cmdPool, vk::Queue &submitQueue);

        void fillBuffer(size_t size, const uint8_t *data, uint32_t offset, vk::Buffer target);
        void fillTexture(size_t size, const uint8_t *data, vk::Extent3D extent, uint32_t layers, vk::Image target);

        // Bookkeeping
        std::unordered_map<Shader, Shader_vkData> shaders;
        std::unordered_map<Buffer, Buffer_vkData> buffers;
        std::unordered_map<Texture, Texture_vkData> textures;
        std::unordered_map<InputSet, InputSet_vkData> inputSets;
        std::unordered_map<RenderPass, RenderPass_vkData> renderPasses;
        std::unordered_map<CommandBuffer, CommandBuffer_vkData> commandBuffers;
        std::unordered_map<Texture, DepthBuffer_vkData> textureDepthBuffers;
        std::unordered_map<Window, DepthBuffer_vkData> windowDepthBuffers;

        struct RecordingData {
            vk::CommandBuffer cmdBuffer;
            RenderPass renderPass;
        } currentRecording;

        void free(Shader shader);
        void free(Buffer buffer);
        void free(Texture texture);
        void free(Window window);
        void free(InputSet inputSet);
        void free(RenderPass renderPass);
        void free(CommandBuffer commandBuffer);
    };
}  // namespace tga