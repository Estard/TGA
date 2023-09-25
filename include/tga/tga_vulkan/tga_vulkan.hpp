#pragma once
#include "tga/tga.hpp"
#include "tga/tga_hash.hpp"
#include "tga_vulkan_WSI.hpp"
#include "tga_vulkan_metadata.hpp"

namespace tga
{
    /** \brief The Interface Implementation over the Vulkan API
     */
    class TGAVulkan final : public Interface {
    public:
        TGAVulkan();
        ~TGAVulkan();

        Shader createShader(const ShaderInfo &shaderInfo) override;
        Buffer createBuffer(const BufferInfo &bufferInfo) override;
        Texture createTexture(const TextureInfo &textureInfo) override;
        Window createWindow(const WindowInfo &windowInfo) override;
        InputSet createInputSet(const InputSetInfo &inputSetInfo) override;
        RenderPass createRenderPass(const RenderPassInfo &renderPassInfo) override;

        ext::TopLevelAccelerationStructure createTopLevelAccelerationStructure(
            const ext::TopLevelAccelerationStructureInfo &TLASInfo) override;
        ext::BottomLevelAccelerationStructure createBottomLevelAccelerationStructure(
            const ext::BottomLevelAccelerationStructureInfo &BLASInfo) override;

        void beginCommandBuffer() override;
        void beginCommandBuffer(CommandBuffer cmdBuffer) override;
        void setRenderPass(RenderPass renderPass, uint32_t frambufferIndex) override;
        void bindVertexBuffer(Buffer buffer) override;
        void bindIndexBuffer(Buffer buffer) override;
        void bindInputSet(InputSet inputSet) override;
        void draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount = 1,
                  uint32_t firstInstance = 0) override;
        void drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t instanceCount = 1,
                         uint32_t firstInstance = 0) override;
        void drawIndirect(Buffer buffer, uint32_t drawCount, size_t offset = 0,
                          uint32_t stride = sizeof(tga::DrawIndirectCommand)) override;
        void drawIndexedIndirect(Buffer buffer, uint32_t drawCount, size_t offset = 0,
                                 uint32_t stride = sizeof(tga::DrawIndexedIndirectCommand)) override;
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
        CommandBuffer endCommandBuffer() override;
        void execute(CommandBuffer commandBuffer) override;

        void updateBuffer(Buffer buffer, uint8_t const *data, size_t dataSize, uint32_t offset) override;
        std::vector<uint8_t> readback(Buffer buffer) override;
        std::vector<uint8_t> readback(Texture texture) override;

        /** \copydoc Interface::backbufferCount(Window window)
         */
        uint32_t backbufferCount(Window window) override;

        /** \copydoc Interface::nextFrame(Window window)
         */
        uint32_t nextFrame(Window window) override;

        /** \copydoc Interface::pollEvents(Window window)
         */
        void pollEvents(Window window) override;

        /** \copydoc Interface::present(Window window)
         */
        void present(Window window) override;

        /** \copydoc Interface::setWindowTitle(Window window, const std::string &title)
         */
        void setWindowTitle(Window window, const std::string &title) override;

        /** \copydoc Interface::windowShouldClose(Window window)
         */
        bool windowShouldClose(Window window) override;

        /** \copydoc Interface::keyDown(Window window)
         */
        bool keyDown(Window window, Key key) override;

        /** \copydoc Interface::mousePosition(Window window)
         */
        std::pair<int, int> mousePosition(Window window) override;

        /** \copydoc Interface::screenResolution()
         */
        std::pair<uint32_t, uint32_t> screenResolution() override;

        void free(Shader shader) override;
        void free(Buffer buffer) override;
        void free(Texture texture) override;
        void free(Window window) override;
        void free(InputSet inputSet) override;
        void free(RenderPass renderPass) override;
        void free(CommandBuffer commandBuffer) override;

    private:
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
        void transitionImageLayout(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout);
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
    };
}  // namespace tga