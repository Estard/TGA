#pragma once
#include "tga/tga.hpp"
#include "tga/tga_hash.hpp"
#include "tga_vulkan_WSI.hpp"
#include "tga_vulkan_metadata.hpp"

namespace tga
{
/** \brief The Interface Implementation over the Vulkan API
 */
struct Interface::InternalState {
    InternalState();
    // Vulkan Stuff
    VulkanWSI wsi;
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugger;
    vk::PhysicalDevice pDevice;
    uint32_t hostMemoryIndex;
    uint32_t deviceMemoryIndex;
    uint32_t renderQueueFamily;
    vk::Device device;
    vk::Queue renderQueue;
    vk::CommandPool cmdPool;

    vkData::Buffer allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    std::tuple<vk::ImageType, vk::ImageViewType, vk::ImageCreateFlags> determineImageTypeInfo(
        const TextureInfo& textureInfo);
    std::tuple<vk::Extent3D, uint32_t> determineImageDimensions(const TextureInfo& textureInfo);
    std::pair<vk::ImageTiling, vk::ImageUsageFlags> determineImageFeatures(vk::Format& format);
    vk::Format findDepthFormat();
    vkData::DepthBuffer createDepthBuffer(uint32_t width, uint32_t height);
    vk::RenderPass makeRenderPass(std::vector<vk::Format> const& colorFormats, ClearOperation clearOps,
                                  vk::ImageLayout layout);
    std::vector<vk::DescriptorSetLayout> decodeInputLayout(InputLayout const& inputLayout);
    vk::Pipeline makeGraphicsPipeline(const RenderPassInfo& renderPassInfo, vk::PipelineLayout pipelineLayout,
                                      vk::RenderPass renderPass);
    std::pair<vk::Pipeline, vk::PipelineBindPoint> makePipeline(const RenderPassInfo& renderPassInfo,
                                                                vk::PipelineLayout pipelineLayout,
                                                                vk::RenderPass renderPass);

    // vk::CommandBuffer beginOneTimeCmdBuffer(vk::CommandPool &cmdPool);
    // void endOneTimeCmdBuffer(vk::CommandBuffer &cmdBuffer, vk::CommandPool &cmdPool, vk::Queue &submitQueue);

    // void fillBuffer(size_t size, const uint8_t *data, uint32_t offset, vk::Buffer target);
    // void fillTexture(size_t size, const uint8_t *data, vk::Extent3D extent, uint32_t layers, vk::Image target);

    // Bookkeeping
    // Note: Windows are stored in WSI
    std::vector<vkData::Shader> shaders;
    std::vector<vkData::Buffer> buffers;
    std::vector<vkData::StagingBuffer> stagingBuffers;
    std::vector<vkData::Texture> textures;
    std::vector<vkData::InputSet> inputSets;
    std::vector<vkData::RenderPass> renderPasses;
    std::vector<vkData::ComputePass> computePasses;
    std::vector<vkData::CommandBuffer> commandBuffers;
    //std::vector<vkData::DepthBuffer> textureDepthBuffers;
    //std::vector<vkData::DepthBuffer> windowDepthBuffers;

    vkData::Shader& getData(Shader);
    vkData::Buffer& getData(Buffer);
    vkData::StagingBuffer& getData(StagingBuffer);
    vkData::Texture& getData(Texture);
    vkData::Window& getData(Window);
    vkData::InputSet& getData(InputSet);
    vkData::RenderPass& getData(RenderPass);
    vkData::ComputePass& getData(ComputePass);
    vkData::CommandBuffer& getData(CommandBuffer);

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