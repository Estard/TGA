#pragma once
#include "tga/tga.hpp"
#include "tga/tga_hash.hpp"
#include "tga_vulkan_WSI.hpp"
#include "tga_vulkan_util.hpp"

namespace tga{

    /** \brief The Interface Implementation over the Vulkan API
    */
    class TGAVulkan : public Interface{
        public:
        TGAVulkan();
        ~TGAVulkan();

        Shader createShader(const ShaderInfo &shaderInfo) override;
        Buffer createBuffer(const BufferInfo &bufferInfo) override;
        Texture createTexture(const TextureInfo &textureInfo) override;
        Window createWindow(const WindowInfo &windowInfo) override;
        InputSet createInputSet(const InputSetInfo &inputSetInfo) override;
        RenderPass createRenderPass(const RenderPassInfo &renderPassInfo) override;

        void beginCommandBuffer() override;
        void setRenderPass(RenderPass renderPass, uint32_t frambufferIndex) override;
        void bindVertexBuffer(Buffer buffer) override;
        void bindIndexBuffer(Buffer buffer) override;
        void bindInputSet(InputSet inputSet) override;
        void draw(uint32_t vertexCount, uint32_t firstVertex) override;
        void drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset) override;
        void dispatch(uint32_t groupCountX,uint32_t groupCountY,uint32_t groupCountZ) override;     
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

        /** \copydoc Interface::present(Window window)
        */
        void present(Window window) override;

        /** \copydoc Interface::setWindowTitel(Window window, const std::string &title)
        */
        void setWindowTitel(Window window, const std::string &title) override;

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

        
        //Vulkan Stuff
        VulkanWSI wsi;
        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugger;
        vk::PhysicalDevice pDevice;
        QueueIndices queueIndices;
        vk::Device device;
        vk::Queue graphicsQueue;
        vk::Queue transferQueue;
        vk::CommandPool transferCmdPool;
        vk::CommandPool graphicsCmdPool;

        const std::vector<const char*> getInstanceExtentensions();
        const std::vector<const char*> getDeviceExtentensions();
        const std::vector<const char*> getLayers();
        vk::PhysicalDeviceFeatures getDeviceFeatures();
        uint32_t findQueueFamily(vk::QueueFlags mask,vk::QueueFlags flags);
        QueueIndices findQueueFamilies();

        vk::Instance createInstance();
        vk::DebugUtilsMessengerEXT createDebugger();
        vk::PhysicalDevice choseGPU();
        vk::Device createDevice();
        vk::CommandPool createCommandPool(uint32_t queueFamily, vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlags());
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
        Buffer_TV allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
        std::pair<vk::ImageTiling, vk::ImageUsageFlags> determineImageFeatures(vk::Format &format);
        vk::Format findDepthFormat();
        DepthBuffer_TV createDepthBuffer(uint32_t width, uint32_t height);
        vk::RenderPass makeRenderPass(vk::Format colorFormat,ClearOperation clearOps, vk::ImageLayout layout);
        std::vector<vk::DescriptorSetLayout> decodeInputLayout(const InputLayout &inputLayout);
        vk::Pipeline makeGraphicsPipeline(const RenderPassInfo &renderPassInfo,vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass);
        std::pair<vk::Pipeline, vk::PipelineBindPoint> makePipeline(const RenderPassInfo &renderPassInfo,vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass);
        

        vk::CommandBuffer beginOneTimeCmdBuffer(vk::CommandPool &cmdPool);
        void endOneTimeCmdBuffer(vk::CommandBuffer &cmdBuffer,vk::CommandPool &cmdPool, vk::Queue &submitQueue);

        void fillBuffer(size_t size,const uint8_t *data,uint32_t offset,vk::Buffer target);
        void transitionImageLayout(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
        void fillTexture(size_t size,const uint8_t *data,uint32_t width, uint32_t height,vk::Image target);

        //Convertes
        vk::BufferUsageFlags determineBufferFlags(tga::BufferUsage usage);
        vk::Format determineImageFormat(tga::Format format);
        std::tuple<vk::Filter, vk::SamplerAddressMode> determineSamplerInfo(const TextureInfo &textureInfo);
        vk::ShaderStageFlagBits determineShaderStage(tga::ShaderType shaderType);
        std::vector<vk::VertexInputAttributeDescription> determineVertexAttributes(const std::vector<VertexAttribute> &attributes);
        vk::PipelineRasterizationStateCreateInfo determineRasterizerState(const RasterizerConfig &config);
        vk::CompareOp determineDepthCompareOp(CompareOperation compareOperation);
        vk::BlendFactor determineBlendFactor(BlendFactor blendFactor);
        vk::PipelineColorBlendAttachmentState determineColorBlending(const RasterizerConfig &config);
        vk::DescriptorType determineDescriptorType(tga::BindingType bindingType);
        vk::AccessFlags layoutToAccessFlags(vk::ImageLayout layout);
        vk::PipelineStageFlags layoutToPipelineStageFlags(vk::ImageLayout layout);
        vk::PipelineStageFlags accessToPipelineStageFlags(vk::AccessFlags accessFlags);

        

        //Bookkeeping
        std::unordered_map<Shader, Shader_TV> shaders;
        std::unordered_map<Buffer, Buffer_TV> buffers;
        std::unordered_map<Texture, Texture_TV> textures;
        std::unordered_map<InputSet, InputSet_TV> inputSets;
        std::unordered_map<RenderPass, RenderPass_TV> renderPasses;
        std::unordered_map<CommandBuffer, CommandBuffer_TV> commandBuffers;
        std::unordered_map<Texture,DepthBuffer_TV> textureDepthBuffers;
        std::unordered_map<Window,DepthBuffer_TV> windowDepthBuffers;

        struct RecordingData{
            vk::CommandBuffer cmdBuffer;
            RenderPass renderPass;
        }currentRecording;
    };
}