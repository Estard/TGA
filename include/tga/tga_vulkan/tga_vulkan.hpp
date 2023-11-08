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

    // Bookkeeping
    template <typename T>
    struct Pool {
        std::vector<T> data;
        std::vector<size_t> freeList;

        size_t insert(T&& obj)
        {
            if (!freeList.empty()) {
                auto idx = freeList.back();
                freeList.pop_back();
                data[idx] = std::move(obj);
                return idx;
            }
            data.push_back(std::move(obj));
            return data.size() - 1;
        }

        void free(size_t idx)
        {
            assert(idx < data.size());
            data[idx] = {};
            freeList.push_back(idx);
        }

        size_t size() const { return data.size(); }

        T& operator[](size_t i) { return data[i]; }
    };

    // Note: Windows are stored in WSI
    Pool<vkData::Shader> shaders;
    Pool<vkData::Buffer> buffers;
    Pool<vkData::StagingBuffer> stagingBuffers;
    Pool<vkData::Texture> textures;
    Pool<vkData::InputSet> inputSets;
    Pool<vkData::RenderPass> renderPasses;
    Pool<vkData::ComputePass> computePasses;
    Pool<vkData::CommandBuffer> commandBuffers;
    Pool<vkData::ext::AccelerationStructure> acclerationStructures;

    vkData::Shader& getData(Shader);
    vkData::Buffer& getData(Buffer);
    vkData::StagingBuffer& getData(StagingBuffer);
    vkData::Texture& getData(Texture);
    vkData::Window& getData(Window);
    vkData::InputSet& getData(InputSet);
    vkData::RenderPass& getData(RenderPass);
    vkData::ComputePass& getData(ComputePass);
    vkData::CommandBuffer& getData(CommandBuffer);
    vkData::ext::AccelerationStructure& getData(ext::TopLevelAccelerationStructure);
    vkData::ext::AccelerationStructure& getData(ext::BottomLevelAccelerationStructure);

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