#ifdef __APPLE__
#define VK_ENABLE_BETA_EXTENSIONS 1
#endif

#include "tga/tga_vulkan/tga_vulkan.hpp"

#include "tga/tga_vulkan/tga_vulkan_debug.hpp"
#include "tga/tga_vulkan/tga_vulkan_extensions.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                          VkDebugUtilsMessageTypeFlagsEXT,
                                                          const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                          void *)
{
    std::cerr << "[VULKAN VALIDATION LAYER]: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

namespace tga
{
namespace /*conversion from tga to vulkan*/
{
    vk::BufferUsageFlags determineBufferFlags(tga::BufferUsage usage)
    {
        if (usage == BufferUsage::undefined) throw std::runtime_error("[TGA Vulkan] Buffer usage is undefined!");

        auto usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

        auto tgaToVkFlag = [&](tga::BufferUsage tgaFlag, vk::BufferUsageFlags vkFlag) {
            if (usage & tgaFlag) usageFlags |= vkFlag;
        };

        tgaToVkFlag(tga::BufferUsage::uniform, vk::BufferUsageFlagBits::eUniformBuffer);
        tgaToVkFlag(tga::BufferUsage::vertex, vk::BufferUsageFlagBits::eVertexBuffer);
        tgaToVkFlag(tga::BufferUsage::index, vk::BufferUsageFlagBits::eIndexBuffer);
        tgaToVkFlag(tga::BufferUsage::storage, vk::BufferUsageFlagBits::eStorageBuffer);
        tgaToVkFlag(tga::BufferUsage::indirect, vk::BufferUsageFlagBits::eIndirectBuffer);
        tgaToVkFlag(tga::BufferUsage::accelerationStructureBuildInput,
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR |
                        vk::BufferUsageFlagBits::eShaderDeviceAddress);

        return usageFlags;
    }

    vk::Format tgaFormatToVkFormat(tga::Format format)
    {
        switch (format) {
            case Format::r8_uint: return vk::Format::eR8Uint;
            case Format::r8_sint: return vk::Format::eR8Sint;
            case Format::r8_srgb: return vk::Format::eR8Srgb;
            case Format::r8_unorm: return vk::Format::eR8Unorm;
            case Format::r8_snorm: return vk::Format::eR8G8Snorm;
            case Format::r8g8_uint: return vk::Format::eR8G8Uint;
            case Format::r8g8_sint: return vk::Format::eR8G8Sint;
            case Format::r8g8_srgb: return vk::Format::eR8G8Srgb;
            case Format::r8g8_unorm: return vk::Format::eR8G8Unorm;
            case Format::r8g8_snorm: return vk::Format::eR8G8Snorm;
            case Format::r8g8b8_uint: return vk::Format::eR8G8B8Uint;
            case Format::r8g8b8_sint: return vk::Format::eR8G8B8Sint;
            case Format::r8g8b8_srgb: return vk::Format::eR8G8B8Srgb;
            case Format::r8g8b8_unorm: return vk::Format::eR8G8B8Unorm;
            case Format::r8g8b8_snorm: return vk::Format::eR8G8B8Snorm;
            case Format::r8g8b8a8_uint: return vk::Format::eR8G8B8A8Uint;
            case Format::r8g8b8a8_sint: return vk::Format::eR8G8B8A8Sint;
            case Format::r8g8b8a8_srgb: return vk::Format::eR8G8B8A8Srgb;
            case Format::r8g8b8a8_unorm: return vk::Format::eR8G8B8A8Unorm;
            case Format::r8g8b8a8_snorm: return vk::Format::eR8G8B8A8Snorm;
            case Format::r32_uint: return vk::Format::eR32Uint;
            case Format::r32_sint: return vk::Format::eR32Sint;
            case Format::r32_sfloat: return vk::Format::eR32Sfloat;
            case Format::r32g32_uint: return vk::Format::eR32G32Uint;
            case Format::r32g32_sint: return vk::Format::eR32G32Sint;
            case Format::r32g32_sfloat: return vk::Format::eR32G32Sfloat;
            case Format::r32g32b32_uint: return vk::Format::eR32G32B32Uint;
            case Format::r32g32b32_sint: return vk::Format::eR32G32B32Sint;
            case Format::r32g32b32_sfloat: return vk::Format::eR32G32B32Sfloat;
            case Format::r32g32b32a32_uint: return vk::Format::eR32G32B32A32Uint;
            case Format::r32g32b32a32_sint: return vk::Format::eR32G32B32A32Sint;
            case Format::r32g32b32a32_sfloat: return vk::Format::eR32G32B32A32Sfloat;
            case Format::r16_sfloat: return vk::Format::eR16Sfloat;
            case Format::r16g16_sfloat: return vk::Format::eR16G16Sfloat;
            case Format::r16g16b16_sfloat: return vk::Format::eR16G16B16Sfloat;
            case Format::r16g16b16a16_sfloat: return vk::Format::eR16G16B16A16Sfloat;
            default: return vk::Format::eUndefined;
        }
    }

    std::vector<vk::VertexInputAttributeDescription> determineVertexAttributes(
        std::vector<VertexAttribute> const& attributes)
    {
        std::vector<vk::VertexInputAttributeDescription> descriptions{};
        for (uint32_t i = 0; i < attributes.size(); i++) {
            descriptions.emplace_back(vk::VertexInputAttributeDescription(
                i, 0, tgaFormatToVkFormat(attributes[i].format), static_cast<uint32_t>(attributes[i].offset)));
        }
        return descriptions;
    }
    vk::PipelineRasterizationStateCreateInfo determineRasterizerState(const RasterizerConfig& config)
    {
        vk::CullModeFlags cullFlags = vk::CullModeFlagBits::eNone;
        vk::PolygonMode polyMode = vk::PolygonMode::eFill;
        vk::FrontFace frontFace = vk::FrontFace::eClockwise;
        if (config.cullMode == CullMode::back) cullFlags = vk::CullModeFlagBits::eBack;
        if (config.cullMode == CullMode::front) cullFlags = vk::CullModeFlagBits::eFront;
        if (config.cullMode == CullMode::all) cullFlags = vk::CullModeFlagBits::eFrontAndBack;
        if (config.polygonMode == PolygonMode::wireframe) polyMode = vk::PolygonMode::eLine;
        if (config.frontFace == FrontFace::counterclockwise) frontFace = vk::FrontFace::eCounterClockwise;
        return {{}, VK_FALSE, VK_FALSE, polyMode, cullFlags, frontFace, VK_FALSE, 0, 0, 0, 1.};
    }

    vk::CompareOp determineDepthCompareOp(CompareOperation compareOperation)
    {
        switch (compareOperation) {
            case CompareOperation::ignore: return vk::CompareOp::eNever;
            case CompareOperation::equal: return vk::CompareOp::eEqual;
            case CompareOperation::greater: return vk::CompareOp::eGreater;
            case CompareOperation::greaterEqual: return vk::CompareOp::eGreaterOrEqual;
            case CompareOperation::less: return vk::CompareOp::eLess;
            case CompareOperation::lessEqual: return vk::CompareOp::eLessOrEqual;
            default: return vk::CompareOp::eAlways;
        }
    }

    vk::BlendFactor determineBlendFactor(BlendFactor blendFactor)
    {
        switch (blendFactor) {
            case BlendFactor::zero: return vk::BlendFactor::eZero;
            case BlendFactor::one: return vk::BlendFactor::eOne;
            case BlendFactor::srcAlpha: return vk::BlendFactor::eSrcAlpha;
            case BlendFactor::dstAlpha: return vk::BlendFactor::eDstAlpha;
            case BlendFactor::oneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
            case BlendFactor::oneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
            default: return vk::BlendFactor::eConstantColor;
        }
    }

    vk::PipelineColorBlendAttachmentState determineColorBlending(const PerPixelOperations& config)
    {
        vk::Bool32 enabled = config.blendEnabled ? VK_TRUE : VK_FALSE;
        vk::BlendFactor srcBlendFac = determineBlendFactor(config.srcBlend);
        vk::BlendFactor dstBlendFac = determineBlendFactor(config.dstBlend);
        vk::BlendFactor srcAlphaBlendFac = determineBlendFactor(config.srcAlphaBlend);
        vk::BlendFactor dstAlphaBlendFac = determineBlendFactor(config.dstAlphaBlend);
        return {enabled,
                srcBlendFac,
                dstBlendFac,
                vk::BlendOp::eAdd,
                srcAlphaBlendFac,
                dstAlphaBlendFac,
                vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                    vk::ColorComponentFlagBits::eA};
    }

}  // namespace

namespace /*init vulkan objects*/
{
    static const std::array<const char *, 1> vulkanLayers = {"VK_LAYER_KHRONOS_validation"};
    vk::Instance createInstance(VulkanWSI const& wsi)
    {
        vk::InstanceCreateFlags iFlags{};

        auto extensions = wsi.getRequiredExtensions();
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#ifdef __APPLE__
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        iFlags = static_cast<vk::InstanceCreateFlagBits>(VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR);
#endif
        vk::ApplicationInfo appInfo("TGA", 1, "TGA", 1, VK_API_VERSION_1_2);
        auto instance = vk::createInstance(vk::InstanceCreateInfo()
                                               .setPApplicationInfo(&appInfo)
                                               .setPEnabledLayerNames(vulkanLayers)
                                               .setPEnabledExtensionNames(extensions)
                                               .setFlags(iFlags));
        loadVkInstanceExtensions(instance);
        return instance;
    }

    vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance& instance)
    {
        using MsgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using MsgType = vk::DebugUtilsMessageTypeFlagBitsEXT;
        return instance.createDebugUtilsMessengerEXT(
            vk::DebugUtilsMessengerCreateInfoEXT()
                .setMessageSeverity(MsgSeverity::eWarning | MsgSeverity::eError)
                .setMessageType(MsgType::eGeneral | MsgType::ePerformance | MsgType::eValidation)
                .setPfnUserCallback(&vulkanDebugCallback));
    }

    vk::PhysicalDevice choseGPU(vk::Instance& instance)
    {
        auto pdevices = instance.enumeratePhysicalDevices();
        auto gpus = pdevices.front();
        for (auto& p : pdevices) {
            auto props = p.getProperties();
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                gpus = p;  // Take the first discrete GPU, aka not an iGPU.
                break;
            }
        }
        return gpus;
    }

    uint32_t findUniversalQueueFamily(vk::PhysicalDevice& gpu)
    {
        const auto& queueFamilies = gpu.getQueueFamilyProperties();
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            auto flags = queueFamilies[i].queueFlags;
            if ((flags & vk::QueueFlagBits::eGraphics) && (flags & vk::QueueFlagBits::eCompute)) return i;
        }

        throw std::runtime_error("GPU does not support Queue with graphics and compute");
    }

    vk::Device createDevice(vk::PhysicalDevice& gpu, uint32_t renderQueueFamily)
    {
        vk::PhysicalDeviceFeatures2 features;
        vk::PhysicalDeviceRayQueryFeaturesKHR rayQueryFeature;
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR asFeature;
        features.pNext = &asFeature;
        asFeature.pNext = &rayQueryFeature;
        gpu.getFeatures2(&features);

        auto extensions = [](bool withRayQuerySupport) -> std::vector<const char *> {
            if (!withRayQuerySupport)
                return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
            else
                return {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                        // Ray Query Extension
                        VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                        // Required by VK_KHR_acceleration_structure
                        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                        // Required for ray queries
                        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
                        // Required by VK_KHR_spirv_1_4
                        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME};
        }(rayQueryFeature.rayQuery);
        if (rayQueryFeature.rayQuery) std::cout << "Vulkan RayQuery extension enabled\n";

#ifdef __APPLE_
        extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

        float queuePriority = 1.0f;
        std::array<vk::DeviceQueueCreateInfo, 1> queueInfos{
            vk::DeviceQueueCreateInfo({}, renderQueueFamily, 1, &queuePriority)};

        auto device = gpu.createDevice(vk::DeviceCreateInfo()
                                           .setPNext(&features)
                                           .setQueueCreateInfos(queueInfos)
                                           .setPEnabledExtensionNames(extensions)
                                           .setPEnabledLayerNames(vulkanLayers));
        loadVkDeviceExtensions(device);
        return device;
    }

}  // namespace

namespace /*helper functions*/
{

    auto layoutTransitionBarrier(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                 vk::ImageAspectFlags aspectFlags)
    {
        return vk::ImageMemoryBarrier()
            .setImage(image)
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setSubresourceRange(vk::ImageSubresourceRange(aspectFlags)
                                     .setLayerCount(VK_REMAINING_ARRAY_LAYERS)
                                     .setLevelCount(VK_REMAINING_MIP_LEVELS));
    }

    uint32_t getBestMemoryOfType(vk::PhysicalDevice& pDevice, vk::MemoryPropertyFlags propertyMask)
    {
        auto memProps = pDevice.getMemoryProperties();
        uint32_t memoryIndex{std::numeric_limits<uint32_t>::max()};
        size_t bestSize{0};

        for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
            auto& memType = memProps.memoryTypes[i];
            if ((memType.propertyFlags & propertyMask) != propertyMask) continue;

            auto heapSize = memProps.memoryHeaps[memType.heapIndex].size;
            if (bestSize >= heapSize) continue;

            bestSize = heapSize;
            memoryIndex = i;
        }
        return memoryIndex;
    }

    constexpr auto hostMemoryProperties =
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    constexpr auto deviceMemoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

    template <typename T>
    T toRawHandle(size_t value)
    {
        T handle;
        value++;
        static_assert(sizeof(handle) == sizeof(value));
        std::memcpy(&handle, &value, sizeof(handle));
        return handle;
    }

    template <typename T>
    size_t dataIndexFromRawHandle(T handle)
    {
        size_t index{};
        static_assert(sizeof(handle) == sizeof(index));
        std::memcpy(&index, &handle, sizeof(index));
        return index - 1;
    }

    struct OneTimeCommand {
        vk::Device& device;
        vk::CommandPool& cmdPool;
        vk::Queue& queue;
        vk::CommandBuffer cmd;
        vk::Fence completionSignal;

        OneTimeCommand(vk::Device& _device, vk::CommandPool& _cmdPool, vk::Queue& _queue)
            : device(_device), cmdPool(_cmdPool), queue(_queue),
              cmd(device.allocateCommandBuffers({cmdPool, vk::CommandBufferLevel::ePrimary, 1})[0]),
              completionSignal(device.createFence({}))
        {
            cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        }

        ~OneTimeCommand()
        {
            cmd.end();
            queue.submit(vk::SubmitInfo().setCommandBuffers(cmd), completionSignal);
            std::ignore = device.waitForFences(1, &completionSignal, true, std::numeric_limits<uint64_t>::max());
            device.freeCommandBuffers(cmdPool, 1, &cmd);
            device.destroyFence(completionSignal);
        }
    };

}  // namespace

Interface::InternalState::InternalState()
    : wsi(VulkanWSI()),                          // WSI determines part of required extensions
      instance(createInstance(wsi)),             // Instance is entry point for Vulkan API
      debugger(createDebugMessenger(instance)),  // A Debugger is nice to have
      pDevice(choseGPU(instance)),               // A Physical Device is typically a GPU
      hostMemoryIndex(getBestMemoryOfType(pDevice, hostMemoryProperties)),      // Shared Memory with driver
      deviceMemoryIndex(getBestMemoryOfType(pDevice, deviceMemoryProperties)),  // basically VRAM
      renderQueueFamily(findUniversalQueueFamily(pDevice)),                     // A queue to rule them all

      device(createDevice(pDevice, renderQueueFamily)), renderQueue(device.getQueue(renderQueueFamily, 0)),
      cmdPool(device.createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer, renderQueueFamily}))
{}

// clang-format off
vkData::Shader& Interface::InternalState::getData(Shader handle) {return shaders[dataIndexFromRawHandle<TgaShader>(handle)]; }
vkData::Buffer& Interface::InternalState::getData(Buffer handle) {return buffers[dataIndexFromRawHandle<TgaBuffer>(handle)]; }
vkData::StagingBuffer& Interface::InternalState::getData(StagingBuffer handle) {return stagingBuffers[dataIndexFromRawHandle<TgaStagingBuffer>(handle)]; }
vkData::Texture& Interface::InternalState::getData(Texture handle) {return textures[dataIndexFromRawHandle<TgaTexture>(handle)]; }
vkData::Window& Interface::InternalState::getData(Window handle) {return wsi.getWindow(handle); }
vkData::InputSet& Interface::InternalState::getData(InputSet handle) {return inputSets[dataIndexFromRawHandle<TgaInputSet>(handle)]; }
vkData::RenderPass& Interface::InternalState::getData(RenderPass handle) {return renderPasses[dataIndexFromRawHandle<TgaRenderPass>(handle)]; }
vkData::ComputePass& Interface::InternalState::getData(ComputePass handle) {return computePasses[dataIndexFromRawHandle<TgaComputePass>(handle)]; }
vkData::CommandBuffer& Interface::InternalState::getData(CommandBuffer handle) {return commandBuffers[dataIndexFromRawHandle<TgaCommandBuffer>(handle)]; }
vkData::ext::AccelerationStructure& Interface::InternalState::getData(ext::TopLevelAccelerationStructure handle){ return acclerationStructures[dataIndexFromRawHandle<TgaTopLevelAccelerationStructure>(handle)];};
vkData::ext::AccelerationStructure& Interface::InternalState::getData(ext::BottomLevelAccelerationStructure handle){ return acclerationStructures[dataIndexFromRawHandle<TgaBottomLevelAccelerationStructure>(handle)];};
// clang-format on

Interface::Interface() : state(std::make_unique<InternalState>()) { std::cout << "TGA Vulkan: Interface opened\n"; }

Interface::~Interface()
{
    auto& device = state->device;
    auto& instance = state->instance;
    auto& debugger = state->debugger;
    auto& cmdPool = state->cmdPool;
    auto& wsi = state->wsi;

    for (size_t i = 0; i < state->shaders.size(); ++i) free(toRawHandle<TgaShader>(i));
    for (size_t i = 0; i < state->buffers.size(); ++i) free(toRawHandle<TgaBuffer>(i));
    for (size_t i = 0; i < state->stagingBuffers.size(); ++i) free(toRawHandle<TgaStagingBuffer>(i));
    for (size_t i = 0; i < state->textures.size(); ++i) free(toRawHandle<TgaTexture>(i));
    for (size_t i = 0; i < state->inputSets.size(); ++i) free(toRawHandle<TgaInputSet>(i));
    for (size_t i = 0; i < state->renderPasses.size(); ++i) free(toRawHandle<TgaRenderPass>(i));
    for (size_t i = 0; i < state->computePasses.size(); ++i) free(toRawHandle<TgaComputePass>(i));
    for (size_t i = 0; i < state->commandBuffers.size(); ++i) free(toRawHandle<TgaCommandBuffer>(i));
    for (size_t i = 0; i < state->acclerationStructures.size(); ++i)
        free(toRawHandle<TgaTopLevelAccelerationStructure>(i));

    while (!wsi.windows.empty()) free(wsi.windows.begin()->first);

    device.waitIdle();
    device.destroy(cmdPool);
    device.destroy();
    if (debugger) instance.destroy(debugger);
    instance.destroy();
    std::cout << "TGA Vulkan: Interface closed\n";
}

/*Interface Methodes*/
Shader Interface::createShader(ShaderInfo const& shaderInfo)
{
    auto& device = state->device;
    auto& shaders = state->shaders;

    vk::ShaderModule module =
        device.createShaderModule({{}, shaderInfo.srcSize, reinterpret_cast<const uint32_t *>(shaderInfo.src)});
    return Shader{toRawHandle<TgaShader>(shaders.insert({module, shaderInfo.type}))};
}

StagingBuffer Interface::createStagingBuffer(StagingBufferInfo const& bufferInfo)
{
    auto& stagingBuffers = state->stagingBuffers;

    auto& device = state->device;
    auto& renderQueueFamily = state->renderQueueFamily;
    auto& hostMemoryIndex = state->hostMemoryIndex;

    auto buffer =
        device.createBuffer(vk::BufferCreateInfo()
                                .setSize(bufferInfo.dataSize)
                                .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc)
                                .setQueueFamilyIndices(renderQueueFamily));

    auto mr = device.getBufferMemoryRequirements(buffer);

    vk::DeviceMemory memory = device.allocateMemory({mr.size, hostMemoryIndex});
    device.bindBufferMemory(buffer, memory, 0);

    auto mapping = device.mapMemory(memory, 0, bufferInfo.dataSize);
    if (bufferInfo.data) std::memcpy(mapping, bufferInfo.data, bufferInfo.dataSize);

    return tga::StagingBuffer{toRawHandle<TgaStagingBuffer>(stagingBuffers.insert({buffer, mapping, memory}))};
}

Buffer Interface::createBuffer(BufferInfo const& bufferInfo)
{
    auto& buffers = state->buffers;

    auto& device = state->device;
    auto renderQueueFamily = state->renderQueueFamily;
    auto deviceMemoryIndex = state->deviceMemoryIndex;

    auto usage = determineBufferFlags(bufferInfo.usage);
    auto buffer = device.createBuffer(
        vk::BufferCreateInfo().setSize(bufferInfo.size).setUsage(usage).setQueueFamilyIndices(renderQueueFamily));

    auto mr = device.getBufferMemoryRequirements(buffer);

    vk::DeviceMemory vkMemory = device.allocateMemory({mr.size, deviceMemoryIndex});
    device.bindBufferMemory(buffer, vkMemory, 0);

    tga::Buffer handle{toRawHandle<TgaBuffer>(buffers.insert({buffer, vkMemory, usage, bufferInfo.size}))};

    if (bufferInfo.srcData) {
        auto& renderQueue = state->renderQueue;
        auto& cmdPool = state->cmdPool;
        auto& staging = state->getData(bufferInfo.srcData);
        OneTimeCommand{device, cmdPool, renderQueue}.cmd.copyBuffer(
            staging.buffer, buffer, vk::BufferCopy().setSize(bufferInfo.size).setDstOffset(bufferInfo.srcDataOffset));
    }

    return handle;
}

Texture Interface::createTexture(TextureInfo const& textureInfo)
{
    auto& textures = state->textures;

    auto& device = state->device;
    auto& pDevice = state->pDevice;
    auto deviceMemoryIndex = state->deviceMemoryIndex;
    auto& cmdPool = state->cmdPool;
    auto& renderQueue = state->renderQueue;

    vk::Format format = tgaFormatToVkFormat(textureInfo.format);

    auto texType = textureInfo.textureType;
    vk::Extent3D extent{textureInfo.width, textureInfo.height,
                        texType == TextureType::_3D ? textureInfo.depthLayers : 1};
    uint32_t layers{(texType != TextureType::_3D && texType != TextureType::_2D) ? textureInfo.depthLayers : 1};

    vk::ImageType imageType{texType == TextureType::_3D ? vk::ImageType::e3D : vk::ImageType::e2D};

    vk::ImageUsageFlags usageFlags{vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst |
                                   vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment |
                                   vk::ImageUsageFlagBits::eStorage};
    // Usage Flags start with everything but some properties might not be available
    {
        auto formatFeatures = pDevice.getFormatProperties(format).optimalTilingFeatures;
        // This is mandatory, since it's otherwise really useles
        if (!(formatFeatures & vk::FormatFeatureFlagBits::eSampledImage))
            throw std::runtime_error("[TGA Vulkan] Chosen Image Format: " + vk::to_string(format) +
                                     " cannot be used as Texture on this System");
        // The following are only required if you want to use them for a specific purpose
        // But if you don't it's fine, if you do, expect to get an error
        if (!(formatFeatures & vk::FormatFeatureFlagBits::eTransferDst)) {
            std::cerr << "[TGA Vulkan] Warning: Image format \"" << vk::to_string(format)
                      << "\" does not support being filled via buffer-to-image-copy on this system\n";
            usageFlags &= ~vk::ImageUsageFlagBits::eTransferDst;
        }
        if (!(formatFeatures & vk::FormatFeatureFlagBits::eColorAttachment)) {
            std::cerr << "[TGA Vulkan] Warning: Image format \"" << vk::to_string(format)
                      << "\" does not support being used as a render target on this system\n";
            usageFlags &= ~vk::ImageUsageFlagBits::eColorAttachment;
        }
        if (!(formatFeatures & vk::FormatFeatureFlagBits::eTransferSrc)) {
            std::cerr << "[TGA Vulkan] Warning: Image format \"" << vk::to_string(format)
                      << "\" does not support being read back from via image-to-buffer-copy on this system\n";
            usageFlags &= ~vk::ImageUsageFlagBits::eTransferSrc;
        }

        if (!(formatFeatures & vk::FormatFeatureFlagBits::eStorageImage)) {
            std::cerr << "[TGA Vulkan] Warning: Image format \"" << vk::to_string(format)
                      << "\" does not support being used as a storage image on this system\n";
            usageFlags &= ~vk::ImageUsageFlagBits::eStorage;
        }
    }

    vk::Image image = device.createImage(vk::ImageCreateInfo(textureInfo.textureType == TextureType::_Cube
                                                                 ? vk::ImageCreateFlagBits::eCubeCompatible
                                                                 : vk::ImageCreateFlags{})
                                             .setImageType(imageType)
                                             .setFormat(format)
                                             .setExtent(extent)
                                             .setMipLevels(1)
                                             .setArrayLayers(layers)
                                             .setUsage(usageFlags)
                                             .setTiling(vk::ImageTiling::eOptimal));
    auto mr = device.getImageMemoryRequirements(image);
    vk::DeviceMemory memory = device.allocateMemory({mr.size, deviceMemoryIndex});
    device.bindImageMemory(image, memory, 0);

    vk::ImageView view = device.createImageView(
        vk::ImageViewCreateInfo()
            .setImage(image)
            .setViewType([&]() {
                switch (textureInfo.textureType) {
                    case TextureType::_2DArray: return vk::ImageViewType::e2DArray;
                    case TextureType::_3D: return vk::ImageViewType::e3D;
                    case TextureType::_Cube: return vk::ImageViewType::eCube;
                    default /* TextureType::_2D*/: return vk::ImageViewType::e2D;
                };
            }())
            .setFormat(format)
            .setSubresourceRange(
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor).setLayerCount(layers).setLevelCount(1)));

    vk::Filter filter{textureInfo.samplerMode == SamplerMode::linear ? vk::Filter::eLinear : vk::Filter::eNearest};
    vk::SamplerAddressMode addressMode{[&]() {
        switch (textureInfo.addressMode) {
            case AddressMode::clampEdge: return vk::SamplerAddressMode::eClampToEdge;
            case AddressMode::clampBorder: return vk::SamplerAddressMode::eClampToBorder;
            case AddressMode::repeat: return vk::SamplerAddressMode::eRepeat;
            case AddressMode::repeatMirror: return vk::SamplerAddressMode::eMirroredRepeat;
            default: return vk::SamplerAddressMode::eClampToBorder;
        }
    }()};

    vk::Sampler sampler = device.createSampler(vk::SamplerCreateInfo()
                                                   .setMinFilter(filter)
                                                   .setMagFilter(filter)
                                                   .setMipmapMode(vk::SamplerMipmapMode::eLinear)
                                                   .setAddressModeU(addressMode)
                                                   .setAddressModeV(addressMode)
                                                   .setAddressModeW(addressMode));

    Texture handle{toRawHandle<TgaTexture>(textures.insert({image, view, memory, sampler, extent, format, {}}))};

    OneTimeCommand createTexture{device, cmdPool, renderQueue};
    if (textureInfo.srcData) {
        auto& stagingData = state->getData(textureInfo.srcData);
        createTexture.cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {},
            layoutTransitionBarrier(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                    vk::ImageAspectFlagBits::eColor));
        createTexture.cmd.copyBufferToImage(stagingData.buffer, image, vk::ImageLayout::eTransferDstOptimal,
                                            vk::BufferImageCopy()
                                                .setBufferOffset(textureInfo.srcDataOffset)
                                                .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layers})
                                                .setImageExtent(extent));
        createTexture.cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eComputeShader, {}, {}, {},
            layoutTransitionBarrier(image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral,
                                    vk::ImageAspectFlagBits::eColor));
    } else {
        createTexture.cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eComputeShader, {}, {}, {},
            layoutTransitionBarrier(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
                                    vk::ImageAspectFlagBits::eColor));
    }
    return handle;
}
Window Interface::createWindow(WindowInfo const& windowInfo)
{
    auto& instance = state->instance;
    auto& pDevice = state->pDevice;
    auto& device = state->device;
    auto& renderQueueFamily = state->renderQueueFamily;
    auto& wsi = state->wsi;

    auto& cmdPool = state->cmdPool;

    auto window = wsi.createWindow(windowInfo, instance, pDevice, device, renderQueueFamily);

    auto& windowData = wsi.getWindow(window);

    windowData.toColorAttachmentTransitionCmds = device.allocateCommandBuffers(
        {cmdPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(windowData.images.size())});

    windowData.toPresentSrcTransitionCmds = device.allocateCommandBuffers(
        {cmdPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(windowData.images.size())});

    for (size_t i = 0; i < windowData.images.size(); ++i) {
        // After acquire operation
        windowData.imageAcquiredSignals.push_back(device.createSemaphore({}));

        auto& toColorAttachmentCmd = windowData.toColorAttachmentTransitionCmds[i];
        toColorAttachmentCmd.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
        toColorAttachmentCmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {},
            layoutTransitionBarrier(windowData.images[i], vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eColorAttachmentOptimal, vk::ImageAspectFlagBits::eColor)
                .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead));
        toColorAttachmentCmd.end();

        // After render operation
        windowData.renderCompletedSignals.push_back(device.createSemaphore({}));

        auto& toPresentSrcCmd = windowData.toPresentSrcTransitionCmds[i];
        toPresentSrcCmd.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
        toPresentSrcCmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, {}, {}, {},
            layoutTransitionBarrier(windowData.images[i], vk::ImageLayout::eColorAttachmentOptimal,
                                    vk::ImageLayout::ePresentSrcKHR, vk::ImageAspectFlagBits::eColor)
                .setDstAccessMask(vk::AccessFlagBits::eMemoryRead));
        toPresentSrcCmd.end();
    }
    return window;
}
InputSet Interface::createInputSet(InputSetInfo const& inputSetInfo)
{
    auto& device = state->device;

    auto& layoutData =
        std::visit([&](auto& pass) -> vkData::Layout& { return state->getData(pass).layout; }, inputSetInfo.targetPass);
    auto& descriptorTypes = layoutData.setDescriptorTypes[inputSetInfo.index];
    auto& setLayouts = layoutData.setLayouts;
    auto& inputSets = state->inputSets;

    vk::PipelineBindPoint bindPoint = std::holds_alternative<tga::RenderPass>(inputSetInfo.targetPass)
                                          ? vk::PipelineBindPoint::eGraphics
                                          : vk::PipelineBindPoint::eCompute;

    std::array<uint32_t, 5> poolSizeCounts{};
    for (auto& binding : inputSetInfo.bindings) {
        switch (descriptorTypes[binding.slot]) {
            case vk::DescriptorType::eUniformBuffer: ++poolSizeCounts[0]; break;
            case vk::DescriptorType::eStorageBuffer: ++poolSizeCounts[1]; break;
            case vk::DescriptorType::eCombinedImageSampler: ++poolSizeCounts[2]; break;
            case vk::DescriptorType::eSampledImage: ++poolSizeCounts[3]; break;
            case vk::DescriptorType::eAccelerationStructureKHR: ++poolSizeCounts[4]; break;
            default: break;
        }
    }

    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.reserve(5);
    for (size_t i = 0; i < poolSizeCounts.size(); ++i) {
        if (!poolSizeCounts[i]) continue;
        switch (i) {
            case 0: poolSizes.push_back({vk::DescriptorType::eUniformBuffer, poolSizeCounts[i]}); break;
            case 1: poolSizes.push_back({vk::DescriptorType::eStorageBuffer, poolSizeCounts[i]}); break;
            case 2: poolSizes.push_back({vk::DescriptorType::eCombinedImageSampler, poolSizeCounts[i]}); break;
            case 3: poolSizes.push_back({vk::DescriptorType::eSampledImage, poolSizeCounts[i]}); break;
            case 4: poolSizes.push_back({vk::DescriptorType::eAccelerationStructureKHR, poolSizeCounts[i]}); break;
        }
    }

    vk::DescriptorPool descriptorPool =
        device.createDescriptorPool(vk::DescriptorPoolCreateInfo().setMaxSets(1).setPoolSizes(poolSizes));

    vk::DescriptorSet descriptorSet =
        device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo()
                                          .setDescriptorPool(descriptorPool)
                                          .setDescriptorSetCount(1)
                                          .setSetLayouts(setLayouts[inputSetInfo.index]))[0];

    std::vector<vk::DescriptorImageInfo> imageInfos;
    imageInfos.reserve(inputSetInfo.bindings.size());
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(inputSetInfo.bindings.size());

    std::vector<vk::WriteDescriptorSet> writeSets;
    for (auto& binding : inputSetInfo.bindings) {
        auto& writeSet = writeSets.emplace_back();
        writeSet.setDstSet(descriptorSet)
            .setDescriptorCount(1)
            .setDstBinding(binding.slot)
            .setDstArrayElement(binding.arrayElement)
            .setDescriptorType(descriptorTypes[binding.slot]);

        if (auto texture = std::get_if<Texture>(&binding.resource)) {
            auto& data = state->getData(*texture);
            auto& imageInfo = imageInfos.emplace_back()
                                  .setImageLayout(vk::ImageLayout::eGeneral)
                                  .setImageView(data.imageView)
                                  .setSampler(data.sampler);
            writeSet.setImageInfo(imageInfo);
        } else if (auto buffer = std::get_if<Buffer>(&binding.resource)) {
            auto& data = state->getData(*buffer);
            auto& bufferInfo = bufferInfos.emplace_back().setBuffer(data.buffer).setRange(data.size).setOffset(0);
            writeSet.setBufferInfo(bufferInfo);
        }
    }
    device.updateDescriptorSets(writeSets, {});

    return tga::InputSet{toRawHandle<TgaInputSet>(
        inputSets.insert({descriptorPool, descriptorSet, bindPoint, layoutData.pipelineLayout, inputSetInfo.index}))};
}

RenderPass Interface::createRenderPass(RenderPassInfo const& renderPassInfo)
{
    auto& device = state->device;
    auto& deviceMemoryIndex = state->deviceMemoryIndex;
    auto& cmdPool = state->cmdPool;
    auto& renderQueue = state->renderQueue;
    auto& renderPasses = state->renderPasses;

    // This should be widely support. If it isn't, change it
    constexpr auto depthFormat = vk::Format::eD32Sfloat;
    // Creates a depth buffer for a render target
    auto initDepthBuffer = [&](vk::Extent2D area) -> vkData::DepthBuffer {
        vk::Image image = device.createImage(
            vk::ImageCreateInfo({}, vk::ImageType::e2D, depthFormat, vk::Extent3D{area.width, area.height, 1})
                .setMipLevels(1)
                .setArrayLayers(1)
                .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                .setTiling(vk::ImageTiling::eOptimal));
        auto mr = device.getImageMemoryRequirements(image);
        vk::DeviceMemory memory = device.allocateMemory({mr.size, deviceMemoryIndex});
        device.bindImageMemory(image, memory, 0);
        vk::ImageView view = device.createImageView(
            vk::ImageViewCreateInfo({}, image, vk::ImageViewType::e2D, depthFormat)
                .setSubresourceRange(
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth).setLayerCount(1).setLevelCount(1)));

        OneTimeCommand{device, cmdPool, renderQueue}.cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, {}, {}, {},
            layoutTransitionBarrier(image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                                    vk::ImageAspectFlagBits::eDepth));

        return {image, view, memory};
    };

    std::vector<vk::AttachmentDescription> attachmentDescs;

    auto pushColorAttachment = [&](vk::Format format, vk::ImageLayout layout) {
        attachmentDescs.push_back(vk::AttachmentDescription({}, format)
                                      .setInitialLayout(layout)
                                      .setFinalLayout(layout)
                                      .setLoadOp([&]() {
                                          switch (renderPassInfo.clearOperations) {
                                              case ClearOperation::color:
                                              case ClearOperation::all: return vk::AttachmentLoadOp::eClear;
                                              default: return vk::AttachmentLoadOp::eLoad;
                                          }
                                      }())
                                      .setStoreOp(vk::AttachmentStoreOp::eStore));
    };

    if (auto texture = std::get_if<Texture>(&renderPassInfo.renderTarget)) {
        auto& textureData = state->getData(*texture);
        if (!textureData.depthBuffer.image)
            textureData.depthBuffer = initDepthBuffer({textureData.extent.width, textureData.extent.height});
        pushColorAttachment(textureData.format, vk::ImageLayout::eGeneral);

    } else if (auto window = std::get_if<Window>(&renderPassInfo.renderTarget)) {
        auto& windowData = state->getData(*window);
        if (!windowData.depthBuffer.image) windowData.depthBuffer = initDepthBuffer(windowData.extent);
        pushColorAttachment(windowData.format, vk::ImageLayout::eColorAttachmentOptimal);
    } else {
        auto& targets = std::get<std::vector<Texture>>(renderPassInfo.renderTarget);
        auto& referenceData = state->getData(targets[0]);
        if (!referenceData.depthBuffer.image)
            referenceData.depthBuffer = initDepthBuffer({referenceData.extent.width, referenceData.extent.height});
        for (auto& target : targets) {
            auto& textureData = state->getData(target);
            pushColorAttachment(textureData.format, vk::ImageLayout::eGeneral);
        }
    }
    attachmentDescs.push_back(vk::AttachmentDescription({}, depthFormat)
                                  .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                                  .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                                  .setLoadOp([&]() {
                                      switch (renderPassInfo.clearOperations) {
                                          case ClearOperation::depth:
                                          case ClearOperation::all: return vk::AttachmentLoadOp::eClear;
                                          default: return vk::AttachmentLoadOp::eLoad;
                                      }
                                  }())
                                  .setStoreOp(vk::AttachmentStoreOp::eStore));

    std::vector<vk::AttachmentReference> colorAttachmentRefs;
    uint32_t numColorAttachments = static_cast<uint32_t>(attachmentDescs.size() - 1);
    colorAttachmentRefs.reserve(numColorAttachments);
    for (uint32_t i = 0; i < numColorAttachments; ++i)
        colorAttachmentRefs.push_back({i, vk::ImageLayout::eColorAttachmentOptimal});
    vk::AttachmentReference depthAttachmentRef{numColorAttachments, vk::ImageLayout::eDepthStencilAttachmentOptimal};
    auto subpassDesc = vk::SubpassDescription()
                           .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                           .setColorAttachments(colorAttachmentRefs)
                           .setPDepthStencilAttachment(&depthAttachmentRef);

    auto subpassDependency =
        vk::SubpassDependency()
            .setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(
                vk::PipelineStageFlagBits::eColorAttachmentOutput)  // TODO: determine safe stage mask for sync
            .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite);

    auto renderPass = device.createRenderPass(vk::RenderPassCreateInfo()
                                                  .setAttachments(attachmentDescs)
                                                  .setSubpasses(subpassDesc)
                                                  .setDependencies(subpassDependency));

    std::vector<vk::Framebuffer> framebuffers;
    vk::Extent2D renderArea;
    size_t numColorAttachmentsPerFrameBuffer{0};

    if (auto texture = std::get_if<Texture>(&renderPassInfo.renderTarget)) {
        numColorAttachmentsPerFrameBuffer = 1;
        auto& textureData = state->getData(*texture);
        renderArea = vk::Extent2D{textureData.extent.width, textureData.extent.height};
        std::array<vk::ImageView, 2> attachments{textureData.imageView, textureData.depthBuffer.imageView};
        framebuffers.push_back(device.createFramebuffer(vk::FramebufferCreateInfo({}, renderPass)
                                                            .setAttachments(attachments)
                                                            .setWidth(textureData.extent.width)
                                                            .setHeight(textureData.extent.height)
                                                            .setLayers(1)));

    } else if (auto window = std::get_if<Window>(&renderPassInfo.renderTarget)) {
        numColorAttachmentsPerFrameBuffer = 1;
        auto& windowData = state->getData(*window);
        renderArea = vk::Extent2D{windowData.extent.width, windowData.extent.height};
        for (auto& imageView : windowData.imageViews) {
            std::array<vk::ImageView, 2> attachments{imageView, windowData.depthBuffer.imageView};
            framebuffers.push_back(device.createFramebuffer(vk::FramebufferCreateInfo({}, renderPass)
                                                                .setAttachments(attachments)
                                                                .setWidth(windowData.extent.width)
                                                                .setHeight(windowData.extent.height)
                                                                .setLayers(1)));
        }

    } else {
        auto& targets = std::get<std::vector<Texture>>(renderPassInfo.renderTarget);
        numColorAttachmentsPerFrameBuffer = targets.size();
        auto& referenceData = state->getData(targets[0]);
        renderArea = vk::Extent2D{referenceData.extent.width, referenceData.extent.height};
        std::vector<vk::ImageView> attachments;
        for (auto& target : targets) {
            auto& textureData = state->getData(target);
            attachments.push_back(textureData.imageView);
        }
        attachments.push_back(referenceData.depthBuffer.imageView);
        framebuffers.push_back(device.createFramebuffer(vk::FramebufferCreateInfo({}, renderPass)
                                                            .setAttachments(attachments)
                                                            .setWidth(referenceData.extent.width)
                                                            .setHeight(referenceData.extent.height)
                                                            .setLayers(1)));
    }

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};
    // The types need to be remembered since it is can't be infered from the input later
    std::vector<std::vector<vk::DescriptorType>> setDescriptorTypes{};
    for (auto& setLayout : renderPassInfo.inputLayout) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};
        std::vector<vk::DescriptorType> bindingTypes{};
        for (uint32_t i = 0; i < setLayout.size(); ++i) {
            auto type = [&]() {
                switch (setLayout[i].type) {
                    case tga::BindingType::sampler: return vk::DescriptorType::eCombinedImageSampler;
                    case tga::BindingType::storageBuffer: return vk::DescriptorType::eStorageBuffer;
                    case tga::BindingType::uniformBuffer: return vk::DescriptorType::eUniformBuffer;
                    case tga::BindingType::storageImage: return vk::DescriptorType::eStorageImage;
                    case tga::BindingType::accelerationStructure: return vk::DescriptorType::eAccelerationStructureKHR;
                };
                return vk::DescriptorType::eCombinedImageSampler;
            }();
            bindings.push_back(
                vk::DescriptorSetLayoutBinding(i, type, setLayout[i].count, vk::ShaderStageFlagBits::eAll));
            bindingTypes.push_back(type);
        }
        setDescriptorTypes.push_back(std::move(bindingTypes));
        descriptorSetLayouts.push_back(device.createDescriptorSetLayout({{}, bindings}));
    }

    auto pipelineLayout = device.createPipelineLayout({{}, descriptorSetLayouts});

    auto& vertexShader = state->getData(renderPassInfo.vertexShader).module;
    auto& fragmentShader = state->getData(renderPassInfo.fragmentShader).module;
    vk::Pipeline pipeline = [&]() {
        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
            vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, vertexShader, "main"),
            vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, fragmentShader, "main")};

        vk::VertexInputBindingDescription vertexBinding{0, uint32_t(renderPassInfo.vertexLayout.vertexSize),
                                                        vk::VertexInputRate::eVertex};
        uint32_t bindingCount = ((renderPassInfo.vertexLayout.vertexSize > 0) ? 1 : 0);
        auto vertexAttributes = determineVertexAttributes(renderPassInfo.vertexLayout.vertexAttributes);
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
            {}, bindingCount, &vertexBinding, uint32_t(vertexAttributes.size()), vertexAttributes.data()};

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly{{}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};

        std::array<vk::DynamicState, 2> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo dynamicState{
            {}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data()};
        vk::Viewport viewport{0, 0, 1, 1, 0, 1};
        vk::Rect2D scissor{{0, 0}, {1, 1}};
        vk::PipelineViewportStateCreateInfo viewportState{{}, 1, &viewport, 1, &scissor};
        auto rasterizer = determineRasterizerState(renderPassInfo.rasterizerConfig);
        vk::PipelineMultisampleStateCreateInfo multisampling{};
        vk::Bool32 depthTest = (renderPassInfo.perPixelOperations.depthCompareOp != CompareOperation::ignore);

        auto depthStencil =
            vk::PipelineDepthStencilStateCreateInfo()
                .setDepthTestEnable(depthTest)
                .setDepthWriteEnable((!renderPassInfo.perPixelOperations.blendEnabled) && depthTest)
                .setDepthCompareOp(determineDepthCompareOp(renderPassInfo.perPixelOperations.depthCompareOp));

        auto colorBlendAttachment = determineColorBlending(renderPassInfo.perPixelOperations);

        uint32_t numBlendAttachemnts = 1;
        if (auto targets = std::get_if<std::vector<Texture>>(&renderPassInfo.renderTarget)) {
            numBlendAttachemnts = static_cast<uint32_t>(targets->size());
        }
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments(numBlendAttachemnts,
                                                                                 colorBlendAttachment);

        vk::PipelineColorBlendStateCreateInfo colorBlending{
            {}, VK_FALSE, vk::LogicOp::eCopy, numBlendAttachemnts, colorBlendAttachments.data(), {0, 0, 0, 0}};

        return device
            .createGraphicsPipeline({}, vk::GraphicsPipelineCreateInfo()
                                            .setStages(shaderStages)
                                            .setPVertexInputState(&vertexInputInfo)
                                            .setPInputAssemblyState(&inputAssembly)
                                            .setPViewportState(&viewportState)
                                            .setPRasterizationState(&rasterizer)
                                            .setPMultisampleState(&multisampling)
                                            .setPDepthStencilState(&depthStencil)
                                            .setPColorBlendState(&colorBlending)
                                            .setPDynamicState(&dynamicState)
                                            .setLayout(pipelineLayout)
                                            .setRenderPass(renderPass))
            .value;
    }();

    return tga::RenderPass{toRawHandle<TgaRenderPass>(renderPasses.insert(
        {pipeline, renderPass, std::move(framebuffers), numColorAttachmentsPerFrameBuffer, renderArea,
         vkData::Layout{pipelineLayout, std::move(descriptorSetLayouts), std::move(setDescriptorTypes)}}))};
}

ComputePass Interface::createComputePass(ComputePassInfo const& computePassInfo)
{
    auto& device = state->device;
    auto& computeShaderModule = state->getData(computePassInfo.computeShader).module;
    auto& computePasses = state->computePasses;

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{};
    // The types need to be remembered since it is can't be infered from the input later
    std::vector<std::vector<vk::DescriptorType>> setDescriptorTypes{};
    for (auto& setLayout : computePassInfo.inputLayout) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};
        std::vector<vk::DescriptorType> bindingTypes{};
        for (uint32_t i = 0; i < setLayout.size(); ++i) {
            auto type = [&]() {
                switch (setLayout[i].type) {
                    case tga::BindingType::sampler: return vk::DescriptorType::eCombinedImageSampler;
                    case tga::BindingType::storageBuffer: return vk::DescriptorType::eStorageBuffer;
                    case tga::BindingType::uniformBuffer: return vk::DescriptorType::eUniformBuffer;
                    case tga::BindingType::storageImage: return vk::DescriptorType::eStorageImage;
                    case tga::BindingType::accelerationStructure: return vk::DescriptorType::eAccelerationStructureKHR;
                };
                return vk::DescriptorType::eCombinedImageSampler;
            }();
            bindings.push_back(
                vk::DescriptorSetLayoutBinding(i, type, setLayout[i].count, vk::ShaderStageFlagBits::eAll));
            bindingTypes.push_back(type);
        }
        setDescriptorTypes.push_back(std::move(bindingTypes));
        descriptorSetLayouts.push_back(device.createDescriptorSetLayout({{}, bindings}));
    }

    auto pipelineLayout = device.createPipelineLayout({{}, descriptorSetLayouts});

    auto pipeline =
        device
            .createComputePipeline({}, {{},
                                        vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute,
                                                                          computeShaderModule, "main"),
                                        pipelineLayout})
            .value;

    ;
    return tga::ComputePass{toRawHandle<TgaComputePass>(computePasses.insert(
        {pipeline, vkData::Layout{pipelineLayout, std::move(descriptorSetLayouts), std::move(setDescriptorTypes)}}))};
}

ext::TopLevelAccelerationStructure Interface::createTopLevelAccelerationStructure(
    ext::TopLevelAccelerationStructureInfo const& TLASInfo)
{
    auto& device = state->device;
    auto& renderQueue = state->renderQueue;
    auto& renderQueueFamily = state->renderQueueFamily;
    auto& cmdPool = state->cmdPool;
    auto& deviceMemoryIndex = state->deviceMemoryIndex;
    auto& acclerationStructures = state->acclerationStructures;

    std::vector<vk::AccelerationStructureInstanceKHR> instances;

    for (auto& instance : TLASInfo.instanceInfos) {
        instances.emplace_back()
            .setTransform(instance.transform)
            .setAccelerationStructureReference(
                device.getAccelerationStructureAddressKHR(state->getData(instance.blas).accelerationStructure))
            .setFlags(vk::GeometryInstanceFlagBitsKHR::eForceOpaque);
    }
    // acinstance.setTransform()
    //.setInstanceCustomIndex()
    //.setAccelerationStructureReference()
    //.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);

    auto instanceData = vk::AccelerationStructureGeometryInstancesDataKHR{}.setData(instances.data());
    // TODO one expects many here
    auto instanceGeometry = vk::AccelerationStructureGeometryKHR{}.setGeometry(instanceData);

    auto maxPrimitives = static_cast<uint32_t>(TLASInfo.instanceInfos.size());

    vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
    buildInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
        .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
        .setGeometries(instanceGeometry)
        .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);

    auto buildSizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                                   buildInfo, maxPrimitives);

    auto acBuffer = device.createBuffer(vk::BufferCreateInfo()
                                            .setSize(buildSizes.accelerationStructureSize)
                                            .setUsage(vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                                                      vk::BufferUsageFlagBits::eShaderDeviceAddress)
                                            .setQueueFamilyIndices(renderQueueFamily));
    auto acMemReq = device.getBufferMemoryRequirements(acBuffer);
    auto acMem = device.allocateMemory({acMemReq.size, deviceMemoryIndex});
    device.bindBufferMemory(acBuffer, acMem, 0);

    auto scratchBuffer = device.createBuffer(
        vk::BufferCreateInfo()
            .setSize(buildSizes.accelerationStructureSize)
            .setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress)
            .setQueueFamilyIndices(renderQueueFamily));
    auto scratchBufferMemReq = device.getBufferMemoryRequirements(acBuffer);
    auto scratchBufferMem = device.allocateMemory({scratchBufferMemReq.size, deviceMemoryIndex});
    device.bindBufferMemory(scratchBuffer, scratchBufferMem, 0);

    buildInfo.setScratchData(device.getBufferAddress(scratchBuffer));

    auto blas = device.createAccelerationStructureKHR(vk::AccelerationStructureCreateInfoKHR{}
                                                          .setBuffer(acBuffer)
                                                          .setSize(buildSizes.accelerationStructureSize)
                                                          .setType(vk::AccelerationStructureTypeKHR::eBottomLevel));

    std::vector<vk::AccelerationStructureBuildRangeInfoKHR const *> rangeInfos;
    // auto rangeInfo = vk::AccelerationStructureBuildRangeInfoKHR{}
    // .setPrimitiveCount(1)
    //                      .setFirstVertex(BLASInfo.firstVertex)
    //                      .setPrimitiveCount(BLASInfo.vertexCount)
    //                      .setPrimitiveOffset(BLASInfo.vertexOffset);

    {
        OneTimeCommand ot{device, cmdPool, renderQueue};
        ot.cmd.buildAccelerationStructuresKHR(buildInfo, rangeInfos);
    }

    auto idx = acclerationStructures.insert({blas, acBuffer, acMem});

    device.destroy(scratchBuffer);
    device.free(scratchBufferMem);
    return tga::ext::TopLevelAccelerationStructure{toRawHandle<TgaTopLevelAccelerationStructure>(idx)};
}

ext::BottomLevelAccelerationStructure Interface::createBottomLevelAccelerationStructure(
    ext::BottomLevelAccelerationStructureInfo const& BLASInfo)
{
    auto& device = state->device;
    auto& renderQueue = state->renderQueue;
    auto& renderQueueFamily = state->renderQueueFamily;
    auto& cmdPool = state->cmdPool;
    auto& deviceMemoryIndex = state->deviceMemoryIndex;
    auto& acclerationStructures = state->acclerationStructures;

    auto& vertexBuffer = state->getData(BLASInfo.vertexBuffer).buffer;
    auto& indexBuffer = state->getData(BLASInfo.indexBuffer).buffer;

    // for geometry description
    auto triangleData = vk::AccelerationStructureGeometryTrianglesDataKHR{}
                            .setIndexType(vk::IndexType::eUint32)
                            .setIndexData(device.getBufferAddress(indexBuffer))
                            .setVertexData(device.getBufferAddress(vertexBuffer))
                            .setVertexFormat(tgaFormatToVkFormat(BLASInfo.vertexPositionFormat))
                            .setVertexStride(BLASInfo.vertexStride)
                            .setMaxVertex(BLASInfo.maxVertex);

    auto geometry = vk::AccelerationStructureGeometryKHR{}
                        .setGeometryType(vk::GeometryTypeKHR::eTriangles)
                        .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                        .setGeometry(triangleData);
    auto maxPrimitives = BLASInfo.vertexCount;

    vk::AccelerationStructureBuildGeometryInfoKHR buildInfo;
    buildInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
        .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
        .setGeometries(geometry)
        .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);

    auto buildSizes = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                                   buildInfo, maxPrimitives);

    auto acBuffer = device.createBuffer(vk::BufferCreateInfo()
                                            .setSize(buildSizes.accelerationStructureSize)
                                            .setUsage(vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                                                      vk::BufferUsageFlagBits::eShaderDeviceAddress)
                                            .setQueueFamilyIndices(renderQueueFamily));
    auto acMemReq = device.getBufferMemoryRequirements(acBuffer);
    auto acMem = device.allocateMemory({acMemReq.size, deviceMemoryIndex});
    device.bindBufferMemory(acBuffer, acMem, 0);

    auto scratchBuffer = device.createBuffer(
        vk::BufferCreateInfo()
            .setSize(buildSizes.accelerationStructureSize)
            .setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress)
            .setQueueFamilyIndices(renderQueueFamily));
    auto scratchBufferMemReq = device.getBufferMemoryRequirements(acBuffer);
    auto scratchBufferMem = device.allocateMemory({scratchBufferMemReq.size, deviceMemoryIndex});
    device.bindBufferMemory(scratchBuffer, scratchBufferMem, 0);

    buildInfo.setScratchData(device.getBufferAddress(scratchBuffer));

    auto blas = device.createAccelerationStructureKHR(vk::AccelerationStructureCreateInfoKHR{}
                                                          .setBuffer(acBuffer)
                                                          .setSize(buildSizes.accelerationStructureSize)
                                                          .setType(vk::AccelerationStructureTypeKHR::eBottomLevel));

    auto rangeInfo = vk::AccelerationStructureBuildRangeInfoKHR{}
                         .setFirstVertex(BLASInfo.firstVertex)
                         .setPrimitiveCount(BLASInfo.vertexCount)
                         .setPrimitiveOffset(BLASInfo.vertexOffset);

    {
        OneTimeCommand ot{device, cmdPool, renderQueue};
        ot.cmd.buildAccelerationStructuresKHR(buildInfo, &rangeInfo);
    }

    auto idx = acclerationStructures.insert({blas, acBuffer, acMem});

    device.destroy(scratchBuffer);
    device.free(scratchBufferMem);
    return tga::ext::BottomLevelAccelerationStructure{toRawHandle<TgaBottomLevelAccelerationStructure>(idx)};
}

CommandBuffer Interface::beginCommandBuffer(CommandBuffer cmdBuffer)
{
    auto& device = state->device;
    auto& commandBuffers = state->commandBuffers;
    auto& cmdPool = state->cmdPool;
    if (!cmdBuffer) {
        cmdBuffer = toRawHandle<TgaCommandBuffer>(
            commandBuffers.insert({device.allocateCommandBuffers({cmdPool, vk::CommandBufferLevel::ePrimary, 1})[0],
                                   device.createFence({vk::FenceCreateFlagBits::eSignaled})}));
    }
    auto& cmdData = state->getData(cmdBuffer);

    std::ignore = device.waitForFences(cmdData.completionFence, true, std::numeric_limits<uint64_t>::max());
    device.resetFences(cmdData.completionFence);
    cmdData.cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});

    return cmdBuffer;
}
void Interface::bindVertexBuffer(CommandBuffer cmdBuffer, Buffer buffer)
{
    state->getData(cmdBuffer).cmdBuffer.bindVertexBuffers(0, {state->getData(buffer).buffer}, {0});
}
void Interface::bindIndexBuffer(CommandBuffer cmdBuffer, Buffer buffer)
{
    state->getData(cmdBuffer).cmdBuffer.bindIndexBuffer(state->getData(buffer).buffer, 0, vk::IndexType::eUint32);
}

void Interface::bindInputSet(CommandBuffer cmdBuffer, InputSet inputSet)
{
    auto& inputSetData = state->getData(inputSet);

    state->getData(cmdBuffer).cmdBuffer.bindDescriptorSets(inputSetData.pipelineBindPoint, inputSetData.pipelineLayout,
                                                           inputSetData.index, 1, &inputSetData.descriptorSet, 0,
                                                           nullptr);
}
void Interface::draw(CommandBuffer cmdBuffer, uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount,
                     uint32_t firstInstance)
{
    state->getData(cmdBuffer).cmdBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}
void Interface::drawIndexed(CommandBuffer cmdBuffer, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset,
                            uint32_t instanceCount, uint32_t firstInstance)
{
    state->getData(cmdBuffer).cmdBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void Interface::drawIndirect(CommandBuffer cmdBuffer, Buffer buffer, uint32_t drawCount, size_t offset, uint32_t stride)
{
    state->getData(cmdBuffer).cmdBuffer.drawIndirect(state->getData(buffer).buffer, offset, drawCount, stride);
}
void Interface::drawIndexedIndirect(CommandBuffer cmdBuffer, Buffer buffer, uint32_t drawCount, size_t offset,
                                    uint32_t stride)
{
    state->getData(cmdBuffer).cmdBuffer.drawIndexedIndirect(state->getData(buffer).buffer, offset, drawCount, stride);
}

void Interface::barrier(CommandBuffer cmdBuffer, PipelineStage srcStage, PipelineStage dstStage)
{
    auto tgaToVkStage = [](tga::PipelineStage stage) -> vk::PipelineStageFlagBits {
        switch (stage) {
            case tga::PipelineStage::TopOfPipe: return vk::PipelineStageFlagBits::eTopOfPipe;
            case tga::PipelineStage::DrawIndirect: return vk::PipelineStageFlagBits::eDrawIndirect;
            case tga::PipelineStage::VertexInput: return vk::PipelineStageFlagBits::eVertexInput;
            case tga::PipelineStage::VertexShader: return vk::PipelineStageFlagBits::eVertexShader;
            case tga::PipelineStage::FragmentShader: return vk::PipelineStageFlagBits::eFragmentShader;
            case tga::PipelineStage::EarlyFragmentTests: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
            case tga::PipelineStage::LateFragmentTests: return vk::PipelineStageFlagBits::eLateFragmentTests;
            case tga::PipelineStage::ColorAttachmentOutput: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
            case tga::PipelineStage::ComputeShader: return vk::PipelineStageFlagBits::eComputeShader;
            case tga::PipelineStage::Transfer: return vk::PipelineStageFlagBits::eTransfer;
            case tga::PipelineStage::BottomOfPipe: return vk::PipelineStageFlagBits::eBottomOfPipe;
            default: return vk::PipelineStageFlagBits::eAllCommands;
        }
    };
    state->getData(cmdBuffer).cmdBuffer.pipelineBarrier(
        tgaToVkStage(srcStage), tgaToVkStage(dstStage), {},
        vk::MemoryBarrier(vk::AccessFlagBits::eMemoryWrite,
                          vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite),
        {}, {});
}

void Interface::dispatch(CommandBuffer cmdBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    state->getData(cmdBuffer).cmdBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
}

void Interface::inlineBufferUpdate(CommandBuffer cmdBuffer, Buffer dst, void const *srcData, uint16_t dataSize,
                                   size_t dstOffset)
{
    state->getData(cmdBuffer).cmdBuffer.updateBuffer(state->getData(dst).buffer, dstOffset, dataSize, srcData);
}
void Interface::bufferUpload(CommandBuffer cmdBuffer, StagingBuffer src, Buffer dst, size_t size, size_t srcOffset,
                             size_t dstOffset)
{
    auto srcBuffer = state->getData(src).buffer;
    auto dstBuffer = state->getData(dst).buffer;
    state->getData(cmdBuffer).cmdBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(srcOffset, dstOffset, size));
}
void Interface::bufferDownload(CommandBuffer cmdBuffer, Buffer src, StagingBuffer dst, size_t size, size_t srcOffset,
                               size_t dstOffset)
{
    auto srcBuffer = state->getData(src).buffer;
    auto dstBuffer = state->getData(dst).buffer;
    state->getData(cmdBuffer).cmdBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(srcOffset, dstOffset, size));
}

void Interface::textureDownload(CommandBuffer cmdBuffer, Texture src, StagingBuffer dst, size_t dstOffset)
{
    auto& imageData = state->getData(src);
    auto dstBuffer = state->getData(dst).buffer;
    state->getData(cmdBuffer).cmdBuffer.copyImageToBuffer(
        imageData.image, vk::ImageLayout::eGeneral, dstBuffer,
        vk::BufferImageCopy(dstOffset)
            .setImageExtent(imageData.extent)
            .setImageSubresource(
                {vk::ImageAspectFlagBits::eColor, VK_REMAINING_MIP_LEVELS, VK_REMAINING_ARRAY_LAYERS}));
}

void Interface::setRenderPass(CommandBuffer cmdBuffer, RenderPass renderPass, uint32_t framebufferIndex,
                              std::array<float, 4> const& colorClearValue, float depthClearValue)
{
    auto& cmdData = state->getData(cmdBuffer);
    auto& renderPassData = state->getData(renderPass);
    if (cmdData.currentRenderPass) cmdData.cmdBuffer.endRenderPass();
    cmdData.currentRenderPass = renderPassData.renderPass;

    std::vector<vk::ClearValue> clearValues(renderPassData.numColorAttachmentsPerFrameBuffer,
                                            vk::ClearColorValue(colorClearValue));
    clearValues.push_back(vk::ClearDepthStencilValue(depthClearValue, 0));

    uint32_t frameIndex = std::min(framebufferIndex, uint32_t(renderPassData.framebuffers.size() - 1));
    cmdData.cmdBuffer.beginRenderPass(
        vk::RenderPassBeginInfo(renderPassData.renderPass, renderPassData.framebuffers[frameIndex])
            .setClearValues(clearValues)
            .setRenderArea(vk::Rect2D().setExtent(renderPassData.area)),
        vk::SubpassContents::eInline);

    cmdData.cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderPassData.pipeline);
    cmdData.cmdBuffer.setViewport(0, vk::Viewport()
                                         .setWidth(static_cast<float>(renderPassData.area.width))
                                         .setHeight(static_cast<float>(renderPassData.area.height))
                                         .setMinDepth(0)
                                         .setMaxDepth(1));
    cmdData.cmdBuffer.setScissor(0, {{{}, renderPassData.area}});
}

void Interface::setComputePass(CommandBuffer cmdBuffer, ComputePass computePass)
{
    state->getData(cmdBuffer).cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
                                                     state->getData(computePass).pipeline);
}

void Interface::endCommandBuffer(CommandBuffer cmdBuffer)
{
    auto& cmdData = state->getData(cmdBuffer);
    if (cmdData.currentRenderPass) {
        cmdData.cmdBuffer.endRenderPass();
        cmdData.currentRenderPass = vk::RenderPass{};
    }
    cmdData.cmdBuffer.end();
}
void Interface::execute(CommandBuffer cmdBuffer)
{
    auto& cmdData = state->getData(cmdBuffer);
    state->renderQueue.submit(vk::SubmitInfo().setCommandBuffers(cmdData.cmdBuffer), cmdData.completionFence);
}

void Interface::waitForCompletion(CommandBuffer cmdBuffer)
{
    auto& device = state->device;
    auto& cmdData = state->getData(cmdBuffer);

    std::ignore = device.waitForFences(cmdData.completionFence, true, std::numeric_limits<uint64_t>::max());
    device.resetFences(cmdData.completionFence);
}

void *Interface::getMapping(StagingBuffer stagingBuffer) { return state->getData(stagingBuffer).mapping; }

uint32_t Interface::backbufferCount(Window window)
{
    return static_cast<uint32_t>(state->wsi.getWindow(window).imageViews.size());
}

uint32_t Interface::nextFrame(Window window)
{
    auto& device = state->device;
    auto& renderQueue = state->renderQueue;
    auto& wsi = state->wsi;
    auto& windowData = state->getData(window);

    wsi.pollEvents(window);

    auto acquireSignal = windowData.nextAcquireSignal;
    windowData.nextAcquireSignal =
        (windowData.nextAcquireSignal + 1) % static_cast<uint32_t>(windowData.imageAcquiredSignals.size());
    auto nextFrameIndex = device
                              .acquireNextImageKHR(windowData.swapchain, std::numeric_limits<uint64_t>::max(),
                                                   windowData.imageAcquiredSignals[acquireSignal])
                              .value;

    vk::PipelineStageFlags waitStage{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    renderQueue.submit(vk::SubmitInfo()
                           .setCommandBuffers(windowData.toColorAttachmentTransitionCmds[nextFrameIndex])
                           .setWaitSemaphores(windowData.imageAcquiredSignals[acquireSignal])
                           .setWaitDstStageMask(waitStage));

    return nextFrameIndex;
}

void Interface::pollEvents(Window window) { state->wsi.pollEvents(window); }

void Interface::present(Window window, uint32_t imageIndex)
{
    auto& windowData = state->getData(window);
    auto& renderQueue = state->renderQueue;

    auto renderSignal = windowData.nextRenderSignal;
    windowData.nextRenderSignal =
        (windowData.nextRenderSignal + 1) % static_cast<uint32_t>(windowData.renderCompletedSignals.size());

    renderQueue.submit(vk::SubmitInfo()
                           .setCommandBuffers(windowData.toPresentSrcTransitionCmds[imageIndex])
                           .setSignalSemaphores(windowData.renderCompletedSignals[renderSignal]));
    auto result = renderQueue.presentKHR(vk::PresentInfoKHR()
                                             .setSwapchains(windowData.swapchain)
                                             .setWaitSemaphores(windowData.renderCompletedSignals[renderSignal])
                                             .setImageIndices(imageIndex));
    if (result != vk::Result::eSuccess) std::cerr << "[TGA Vulkan] Warning: Window Surface has become suboptimal\n";
}

void Interface::setWindowTitle(Window window, const std::string& title)
{
    state->wsi.setWindowTitle(window, title.c_str());
}

bool Interface::windowShouldClose(Window window) { return state->wsi.windowShouldClose(window); }

bool Interface::keyDown(Window window, Key key) { return state->wsi.keyDown(window, key); }

std::pair<int, int> Interface::mousePosition(Window window) { return state->wsi.mousePosition(window); }

std::pair<uint32_t, uint32_t> Interface::screenResolution() { return state->wsi.screenResolution(); }

void Interface::free(Shader shader)
{
    if (!shader) return;
    auto& device = state->device;
    auto& data = state->getData(shader);
    if (!data.module) return;

    device.waitIdle();
    device.destroy(data.module);
    state->shaders.free(dataIndexFromRawHandle(shader));
}

void Interface::free(StagingBuffer buffer)
{
    if (!buffer) return;
    auto& device = state->device;
    auto& data = state->getData(buffer);
    if (!data.buffer) return;

    device.waitIdle();
    device.destroy(data.buffer);
    device.unmapMemory(data.memory);
    device.free(data.memory);
    state->stagingBuffers.free(dataIndexFromRawHandle(buffer));
}

void Interface::free(Buffer buffer)
{
    if (!buffer) return;
    auto& device = state->device;
    auto& data = state->getData(buffer);
    if (!data.buffer) return;

    device.waitIdle();
    device.destroy(data.buffer);
    device.free(data.memory);
    state->buffers.free(dataIndexFromRawHandle(buffer));
}
void Interface::free(Texture texture)
{
    if (!texture) return;
    auto& device = state->device;
    auto& data = state->getData(texture);

    if (!data.image) return;

    device.waitIdle();
    if (data.depthBuffer.image) {
        device.destroy(data.depthBuffer.imageView);
        device.destroy(data.depthBuffer.image);
        device.free(data.depthBuffer.memory);
        data.depthBuffer = {};
    }

    device.destroy(data.sampler);
    device.destroy(data.imageView);
    device.destroy(data.image);
    device.free(data.memory);
    state->textures.free(dataIndexFromRawHandle(texture));
}
void Interface::free(Window window)
{
    if (!window) return;
    auto& instance = state->instance;
    auto& device = state->device;
    auto& cmdPool = state->cmdPool;
    auto& wsi = state->wsi;
    auto& windowData = state->getData(window);

    device.waitIdle();
    if (windowData.depthBuffer.image) {
        device.destroy(windowData.depthBuffer.imageView);
        device.destroy(windowData.depthBuffer.image);
        device.free(windowData.depthBuffer.memory);
        windowData.depthBuffer = {};
    }

    device.freeCommandBuffers(cmdPool, windowData.toColorAttachmentTransitionCmds);
    device.freeCommandBuffers(cmdPool, windowData.toPresentSrcTransitionCmds);

    wsi.free(window, instance, device);
}
void Interface::free(InputSet inputSet)
{
    if (!inputSet) return;
    auto& device = state->device;
    auto& data = state->getData(inputSet);
    if (!data.descriptorPool) return;

    device.waitIdle();
    device.destroy(data.descriptorPool);
    state->inputSets.free(dataIndexFromRawHandle(inputSet));
}
void Interface::free(RenderPass renderPass)
{
    if (!renderPass) return;
    auto& device = state->device;
    auto& data = state->getData(renderPass);
    if (!data.pipeline) return;

    device.waitIdle();
    for (auto& fb : data.framebuffers) device.destroy(fb);
    device.destroy(data.renderPass);

    for (auto& sl : data.layout.setLayouts) device.destroy(sl);
    device.destroy(data.pipeline);
    device.destroy(data.layout.pipelineLayout);
    state->renderPasses.free(dataIndexFromRawHandle(renderPass));
}

void Interface::free(ComputePass computePass)
{
    if (!computePass) return;
    auto& device = state->device;
    auto& data = state->getData(computePass);
    if (!data.pipeline) return;

    device.waitIdle();
    for (auto& sl : data.layout.setLayouts) device.destroy(sl);
    device.destroy(data.pipeline);
    device.destroy(data.layout.pipelineLayout);
    state->computePasses.free(dataIndexFromRawHandle(computePass));
}

void Interface::free(CommandBuffer commandBuffer)
{
    if (!commandBuffer) return;
    auto& device = state->device;
    auto& cmdPool = state->cmdPool;
    auto& data = state->getData(commandBuffer);
    if (!data.cmdBuffer) return;
    device.waitIdle();
    device.freeCommandBuffers(cmdPool, {data.cmdBuffer});
    device.destroy(data.completionFence);
    state->commandBuffers.free(dataIndexFromRawHandle(commandBuffer));
}

void Interface::free(ext::TopLevelAccelerationStructure acStructure)
{
    if (!acStructure) return;
    auto& device = state->device;
    auto& data = state->getData(acStructure);
    if (!data.accelerationStructure) return;
    device.waitIdle();
    device.destroyAccelerationStructureKHR(data.accelerationStructure);
    device.destroy(data.buffer);
    device.free(data.memory);
    state->acclerationStructures.free(dataIndexFromRawHandle(acStructure));
}
void Interface::free(ext::BottomLevelAccelerationStructure acStructure)
{
    if (!acStructure) return;
    auto& device = state->device;
    auto& data = state->getData(acStructure);
    if (!data.accelerationStructure) return;
    device.waitIdle();
    device.destroyAccelerationStructureKHR(data.accelerationStructure);
    device.destroy(data.buffer);
    device.free(data.memory);
    state->acclerationStructures.free(dataIndexFromRawHandle(acStructure));
}

}  // namespace tga