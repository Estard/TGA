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

    vk::Format determineImageFormat(tga::Format format)
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

    // std::tuple<vk::Filter, vk::SamplerAddressMode> determineSamplerInfo(const TextureInfo& textureInfo)
    // {
    //     auto filter = vk::Filter::eNearest;
    //     if (textureInfo.samplerMode == SamplerMode::linear) filter = vk::Filter::eLinear;
    //     vk::SamplerAddressMode addressMode{vk::SamplerAddressMode::eClampToBorder};
    //     switch (textureInfo.addressMode) {
    //         case AddressMode::clampEdge: addressMode = vk::SamplerAddressMode::eClampToEdge; break;
    //         case AddressMode::clampBorder: addressMode = vk::SamplerAddressMode::eClampToBorder; break;
    //         case AddressMode::repeat: addressMode = vk::SamplerAddressMode::eRepeat; break;
    //         case AddressMode::repeatMirror: addressMode = vk::SamplerAddressMode::eMirroredRepeat; break;
    //     }
    //     return {filter, addressMode};
    // }

    // vk::ShaderStageFlagBits determineShaderStage(tga::ShaderType shaderType)
    // {
    //     switch (shaderType) {
    //         case ShaderType::vertex: return vk::ShaderStageFlagBits::eVertex;
    //         case ShaderType::fragment: return vk::ShaderStageFlagBits::eFragment;
    //         case ShaderType::compute: return vk::ShaderStageFlagBits::eCompute;
    //         default: return vk::ShaderStageFlagBits::eAllGraphics;
    //     }
    // }

    std::vector<vk::VertexInputAttributeDescription> determineVertexAttributes(
        std::vector<VertexAttribute> const& attributes)
    {
        std::vector<vk::VertexInputAttributeDescription> descriptions{};
        for (uint32_t i = 0; i < attributes.size(); i++) {
            descriptions.emplace_back(vk::VertexInputAttributeDescription(
                i, 0, determineImageFormat(attributes[i].format), static_cast<uint32_t>(attributes[i].offset)));
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

    // vk::DescriptorType determineDescriptorType(tga::BindingType bindingType)
    // {
    //     switch (bindingType) {
    //         case BindingType::uniformBuffer: return vk::DescriptorType::eUniformBuffer;
    //         case BindingType::sampler: return vk::DescriptorType::eCombinedImageSampler;
    //         case BindingType::storageBuffer: return vk::DescriptorType::eStorageBuffer;
    //         default: return vk::DescriptorType::eInputAttachment;
    //     }
    // }

    // vk::AccessFlags layoutToAccessFlags(vk::ImageLayout layout)
    // {
    //     switch (layout) {
    //         case vk::ImageLayout::eUndefined: return {};
    //         case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
    //         case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
    //         case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
    //         case vk::ImageLayout::eColorAttachmentOptimal:
    //             return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    //         case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits::eMemoryRead;
    //         case vk::ImageLayout::eGeneral: return vk::AccessFlagBits::eShaderRead |
    //         vk::AccessFlagBits::eShaderWrite; case vk::ImageLayout::eDepthStencilAttachmentOptimal:
    //             return vk::AccessFlagBits::eDepthStencilAttachmentRead |
    //                    vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    //         case vk::ImageLayout::eDepthAttachmentOptimal:
    //             return vk::AccessFlagBits::eDepthStencilAttachmentRead |
    //                    vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    //         case vk::ImageLayout::eStencilAttachmentOptimal:
    //             return vk::AccessFlagBits::eDepthStencilAttachmentRead |
    //                    vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    //         default: throw std::runtime_error("[TGA Vulkan] Layout to AccessFlags transition not supported"); ;
    //     }
    // }

    // TODO use?
    // vk::PipelineStageFlags layoutToPipelineStageFlags(vk::ImageLayout layout)
    // {
    //     switch (layout) {
    //         case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
    //         case vk::ImageLayout::eTransferDstOptimal: return vk::PipelineStageFlagBits::eTransfer;
    //         case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits::eTransfer;
    //         case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eVertexShader;
    //         case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
    //         case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits::eAllGraphics;
    //         case vk::ImageLayout::eGeneral: return vk::PipelineStageFlagBits::eAllCommands;
    //         case vk::ImageLayout::eDepthStencilAttachmentOptimal: return
    //         vk::PipelineStageFlagBits::eEarlyFragmentTests; case vk::ImageLayout::eDepthAttachmentOptimal: return
    //         vk::PipelineStageFlagBits::eEarlyFragmentTests; case vk::ImageLayout::eStencilAttachmentOptimal: return
    //         vk::PipelineStageFlagBits::eEarlyFragmentTests; default: throw std::runtime_error("[TGA Vulkan] Layout to
    //         PipelineStageFlags transition not supported");
    //     }
    // }

    // vk::PipelineStageFlags accessToPipelineStageFlags(vk::AccessFlags accessFlags)
    // {
    //     if (accessFlags == vk::AccessFlags{}) return vk::PipelineStageFlagBits::eTopOfPipe;
    //     vk::PipelineStageFlags pipelineStageFlags{};
    //     if ((accessFlags & vk::AccessFlagBits::eIndirectCommandRead) == vk::AccessFlagBits::eIndirectCommandRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eDrawIndirect;
    //     if ((accessFlags & vk::AccessFlagBits::eIndexRead) == vk::AccessFlagBits::eIndexRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexInput;
    //     if ((accessFlags & vk::AccessFlagBits::eVertexAttributeRead) == vk::AccessFlagBits::eVertexAttributeRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexInput;
    //     if ((accessFlags & vk::AccessFlagBits::eUniformRead) == vk::AccessFlagBits::eUniformRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexShader;
    //     if ((accessFlags & vk::AccessFlagBits::eShaderRead) == vk::AccessFlagBits::eShaderRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexShader;
    //     if ((accessFlags & vk::AccessFlagBits::eShaderWrite) == vk::AccessFlagBits::eShaderWrite)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexShader;
    //     if ((accessFlags & vk::AccessFlagBits::eInputAttachmentRead) == vk::AccessFlagBits::eInputAttachmentRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eFragmentShader;
    //     if ((accessFlags & vk::AccessFlagBits::eColorAttachmentRead) == vk::AccessFlagBits::eColorAttachmentRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
    //     if ((accessFlags & vk::AccessFlagBits::eColorAttachmentWrite) == vk::AccessFlagBits::eColorAttachmentWrite)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
    //     if ((accessFlags & vk::AccessFlagBits::eDepthStencilAttachmentRead) ==
    //         vk::AccessFlagBits::eDepthStencilAttachmentRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
    //     if ((accessFlags & vk::AccessFlagBits::eDepthStencilAttachmentWrite) ==
    //         vk::AccessFlagBits::eDepthStencilAttachmentWrite)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
    //     if ((accessFlags & vk::AccessFlagBits::eTransferRead) == vk::AccessFlagBits::eTransferRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eTransfer;
    //     if ((accessFlags & vk::AccessFlagBits::eTransferWrite) == vk::AccessFlagBits::eTransferWrite)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eTransfer;
    //     if ((accessFlags & vk::AccessFlagBits::eHostRead) == vk::AccessFlagBits::eHostRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eHost;
    //     if ((accessFlags & vk::AccessFlagBits::eHostWrite) == vk::AccessFlagBits::eHostWrite)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eHost;
    //     if ((accessFlags & vk::AccessFlagBits::eMemoryRead) == vk::AccessFlagBits::eMemoryRead)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eBottomOfPipe;
    //     if ((accessFlags & vk::AccessFlagBits::eMemoryWrite) == vk::AccessFlagBits::eMemoryWrite)
    //         pipelineStageFlags |= vk::PipelineStageFlagBits::eAllGraphics;
    //     return pipelineStageFlags;
    // }

}  // namespace

namespace /*init vulkan objects*/
{
    static const std::array<const char *, 1> vulkanLayers = {"VK_LAYER_KHRONOS_validation"};
    vk::Instance createInstance(VulkanWSI const& wsi)
    {
        auto extensions = wsi.getRequiredExtensions();
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        vk::ApplicationInfo appInfo("TGA", 1, "TGA", 1, VK_API_VERSION_1_2);
        auto instance = vk::createInstance(vk::InstanceCreateInfo()
                                               .setPApplicationInfo(&appInfo)
                                               .setPEnabledLayerNames(vulkanLayers)
                                               .setPEnabledExtensionNames(extensions)

        );
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
            auto heapSize = memProps.memoryHeaps[memType.heapIndex].size;
            if ((memType.propertyFlags & propertyMask) != propertyMask) continue;
            if (bestSize > heapSize) continue;
            memoryIndex = i;
            bestSize = heapSize;
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
// clang-format on

Interface::Interface() : state(std::make_unique<InternalState>())
{
    std::cout << "TGA Vulkan Created\n";
    createBottomLevelAccelerationStructure({});
}

Interface::~Interface()
{
    auto& device = state->device;
    auto& instance = state->instance;
    auto& debugger = state->debugger;
    auto& cmdPool = state->cmdPool;
    auto& wsi = state->wsi;

    for (size_t i = 0; i < state->shaders.size(); ++i) free(toRawHandle<TgaShader>(i + 1));
    for (size_t i = 0; i < state->buffers.size(); ++i) free(toRawHandle<TgaBuffer>(i + 1));
    for (size_t i = 0; i < state->stagingBuffers.size(); ++i) free(toRawHandle<TgaStagingBuffer>(i + 1));
    for (size_t i = 0; i < state->textures.size(); ++i) free(toRawHandle<TgaTexture>(i + 1));
    for (size_t i = 0; i < state->inputSets.size(); ++i) free(toRawHandle<TgaInputSet>(i + 1));
    for (size_t i = 0; i < state->renderPasses.size(); ++i) free(toRawHandle<TgaRenderPass>(i + 1));
    for (size_t i = 0; i < state->computePasses.size(); ++i) free(toRawHandle<TgaComputePass>(i + 1));
    for (size_t i = 0; i < state->commandBuffers.size(); ++i) free(toRawHandle<TgaCommandBuffer>(i + 1));

    while (!wsi.windows.empty()) free(wsi.windows.begin()->first);

    device.waitIdle();
    device.destroy(cmdPool);
    device.destroy();
    if (debugger) instance.destroy(debugger);
    instance.destroy();
}

/*Interface Methodes*/
Shader Interface::createShader(ShaderInfo const& shaderInfo)
{
    auto& device = state->device;
    auto& shaders = state->shaders;

    vk::ShaderModule module =
        device.createShaderModule({{}, shaderInfo.srcSize, reinterpret_cast<const uint32_t *>(shaderInfo.src)});
    shaders.push_back({module, shaderInfo.type});
    return Shader{toRawHandle<TgaShader>(shaders.size())};
}

StagingBuffer Interface::createStagingBuffer(StagingBufferInfo const& bufferInfo)
{
    auto& stagingBuffers = state->stagingBuffers;

    auto& device = state->device;
    auto renderQueueFamily = state->renderQueueFamily;
    auto hostMemoryIndex = state->hostMemoryIndex;

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

    stagingBuffers.push_back({buffer, mapping, memory});
    return tga::StagingBuffer{toRawHandle<TgaStagingBuffer>(stagingBuffers.size())};
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

    buffers.push_back({buffer, vkMemory, usage, mr.size});
    tga::Buffer handle{toRawHandle<TgaBuffer>(buffers.size())};

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

    vk::Format format = determineImageFormat(textureInfo.format);

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

    textures.push_back({image, view, memory, sampler, extent, format, {}});
    Texture handle{toRawHandle<TgaTexture>(textures.size())};

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
    // auto& data = wsi.getWindow(window);
    // OneTimeCommand ot{device, cmdPool, renderQueue};
    // for (auto& image : data.images)
    // ot.cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, {}, {}, {},
    //    layoutTransitionBarrier(image, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
    //    vk::ImageAspectFlagBits::eColor));
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

    inputSets.push_back({descriptorPool, descriptorSet, bindPoint, layoutData.pipelineLayout, inputSetInfo.index});
    return tga::InputSet{toRawHandle<TgaInputSet>(inputSets.size())};
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
            layoutTransitionBarrier(image, vk::ImageLayout::eUndefined,
                                    vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal,
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
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
            .setDstAccessMask(vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite);

    auto renderPass = device.createRenderPass(vk::RenderPassCreateInfo()
                                                  .setAttachments(attachmentDescs)
                                                  .setSubpasses(subpassDesc)
                                                  .setDependencies(subpassDependency));

    std::vector<vk::Framebuffer> framebuffers;
    vk::Extent2D renderArea;

    if (auto texture = std::get_if<Texture>(&renderPassInfo.renderTarget)) {
        auto& textureData = state->getData(*texture);
        renderArea = vk::Extent2D{textureData.extent.width, textureData.extent.height};
        std::array<vk::ImageView, 2> attachments{textureData.imageView, textureData.depthBuffer.imageView};
        framebuffers.push_back(device.createFramebuffer(vk::FramebufferCreateInfo({}, renderPass)
                                                            .setAttachments(attachments)
                                                            .setWidth(textureData.extent.width)
                                                            .setHeight(textureData.extent.height)
                                                            .setLayers(1)));

    } else if (auto window = std::get_if<Window>(&renderPassInfo.renderTarget)) {
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

    renderPasses.push_back(
        {pipeline, renderPass, std::move(framebuffers), renderArea,
         vkData::Layout{pipelineLayout, std::move(descriptorSetLayouts), std::move(setDescriptorTypes)}});
    return tga::RenderPass{toRawHandle<TgaRenderPass>(renderPasses.size())};
}

ext::TopLevelAccelerationStructure createTopLevelAccelerationStructure(
    ext::TopLevelAccelerationStructureInfo const& TLASInfo)
{
    /*
    1. Get Size info
    2. Allocate Memory
    3. Create Host Handle
    4. Build Structure
    */

    return {};
}

ext::BottomLevelAccelerationStructure Interface::createBottomLevelAccelerationStructure(
    ext::BottomLevelAccelerationStructureInfo const& BLASInfo)
{
    /*
    1. Get Size info
    2. Allocate Memory
    3. Create Host Handle
    4. Build Structure
    */
    // auto buildSize =
    // device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,{});

    // buildSize.accelerationStructureSize

    // vk::AccelerationStructureCreateInfoKHR();
    // auto test = device.createAccelerationStructureKHR({});

    return {};
}

// void Interface::beginCommandBuffer()
// {
//     if (currentRecording.cmdBuffer) throw std::runtime_error("[TGA Vulkan] Another Commandbuffer is still
//     recording!"); currentRecording.cmdBuffer = device.allocateCommandBuffers({cmdPool,
//     vk::CommandBufferLevel::ePrimary, 1})[0];
//     currentRecording.cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
// }
CommandBuffer Interface::beginCommandBuffer(CommandBuffer cmdBuffer)
{
    auto& device = state->device;
    auto& commandBuffers = state->commandBuffers;
    auto& cmdPool = state->cmdPool;
    if (!cmdBuffer) {
        commandBuffers.push_back({device.allocateCommandBuffers({cmdPool, vk::CommandBufferLevel::ePrimary, 1})[0],
                                  device.createFence({vk::FenceCreateFlagBits::eSignaled})});
        cmdBuffer = toRawHandle<TgaCommandBuffer>(commandBuffers.size());
    }
    auto& cmdData = state->getData(cmdBuffer);

    std::ignore = device.waitForFences(cmdData.completionFence, true, std::numeric_limits<uint32_t>::max());
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
void Interface::dispatch(CommandBuffer cmdBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    state->getData(cmdBuffer).cmdBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
}

void Interface::setRenderPass(CommandBuffer cmdBuffer, RenderPass renderPass, uint32_t framebufferIndex)
{
    auto& cmdData = state->getData(cmdBuffer);
    auto& renderPassData = state->getData(renderPass);
    if (cmdData.currentRenderPass) cmdData.cmdBuffer.endRenderPass();
    cmdData.currentRenderPass = renderPassData.renderPass;

    std::array<vk::ClearValue, 2> clearValues = {
        vk::ClearColorValue(/*0,0,0,0*/),
        vk::ClearDepthStencilValue(1.f, 0),
    };

    uint32_t frameIndex = std::min(framebufferIndex, uint32_t(renderPassData.framebuffers.size() - 1));
    cmdData.cmdBuffer.beginRenderPass(
        vk::RenderPassBeginInfo(renderPassData.renderPass, renderPassData.framebuffers[frameIndex])
            .setClearValues(clearValues)
            .setRenderArea(vk::Rect2D().setExtent(renderPassData.area)),
        vk::SubpassContents::eInline);

    cmdData.cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderPassData.pipeline);
    cmdData.cmdBuffer.setViewport(0, vk::Viewport()
                                         .setWidth(renderPassData.area.width)
                                         .setHeight(renderPassData.area.height)
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


void* Interface::getMapping(StagingBuffer stagingBuffer){
    return state->getData(stagingBuffer).mapping;
}

// void Interface::updateBuffer(Buffer buffer, uint8_t const *data, size_t dataSize, uint32_t offset)
// {
//     auto& handle = buffers[buffer];
//     fillBuffer(dataSize, data, offset, handle.buffer);
// }

// std::vector<uint8_t> Interface::readback(Buffer buffer)
// {
//     auto& handle = buffers[buffer];
//     std::vector<uint8_t> rbBuffer{};
//     rbBuffer.resize(handle.size);

//     auto staging = allocateBuffer(handle.size, vk::BufferUsageFlagBits::eTransferDst,
//                                   vk::MemoryPropertyFlagBits::eHostVisible |
//                                   vk::MemoryPropertyFlagBits::eHostCoherent);

//     auto copyCmdBuffer = beginOneTimeCmdBuffer(transferCmdPool);
//     vk::BufferCopy region{0, 0, handle.size};
//     copyCmdBuffer.copyBuffer(handle.buffer, staging.buffer, {region});
//     endOneTimeCmdBuffer(copyCmdBuffer, transferCmdPool, renderQueue);
//     auto mapping = device.mapMemory(staging.memory, 0, handle.size, {});
//     std::memcpy(rbBuffer.data(), mapping, handle.size);
//     device.unmapMemory(staging.memory);
//     device.destroy(staging.buffer);
//     device.free(staging.memory);
//     return rbBuffer;
// }

// std::vector<uint8_t> Interface::readback(Texture texture)
// {
//     auto& handle = textures[texture];
//     auto mr = device.getImageMemoryRequirements(handle.image);
//     std::vector<uint8_t> rbBuffer{};
//     rbBuffer.resize(mr.size);

//     auto staging = allocateBuffer(mr.size, vk::BufferUsageFlagBits::eTransferDst,
//                                   vk::MemoryPropertyFlagBits::eHostVisible |
//                                   vk::MemoryPropertyFlagBits::eHostCoherent);

//     auto copyCmdBuffer = beginOneTimeCmdBuffer(cmdPool);
//     transitionImageLayout(copyCmdBuffer, handle.image, vk::ImageLayout::eGeneral,
//     vk::ImageLayout::eTransferSrcOptimal); vk::BufferImageCopy region{0, 0, 0, {vk::ImageAspectFlagBits::eColor, 0,
//     0, 1}, {}, handle.extent}; copyCmdBuffer.copyImageToBuffer(handle.image, vk::ImageLayout::eTransferSrcOptimal,
//     staging.buffer, {region}); transitionImageLayout(copyCmdBuffer, handle.image,
//     vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral); endOneTimeCmdBuffer(copyCmdBuffer, cmdPool,
//     renderQueue); auto mapping = device.mapMemory(staging.memory, 0, mr.size, {}); std::memcpy(rbBuffer.data(),
//     mapping, mr.size); device.unmapMemory(staging.memory); device.destroy(staging.buffer);
//     device.free(staging.memory);
//     return rbBuffer;
// }

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
    windowData.nextAcquireSignal = (windowData.nextAcquireSignal + 1) % windowData.imageAcquiredSignals.size();
    auto nextFrameIndex =
        device.acquireNextImageKHR(windowData.swapchain, 0, windowData.imageAcquiredSignals[acquireSignal]).value;

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
    windowData.nextRenderSignal = (windowData.nextRenderSignal + 1) % windowData.renderCompletedSignals.size();

    renderQueue.submit(vk::SubmitInfo()
                           .setCommandBuffers(windowData.toPresentSrcTransitionCmds[imageIndex])
                           .setSignalSemaphores(windowData.renderCompletedSignals[renderSignal]));
    auto result = renderQueue.presentKHR(vk::PresentInfoKHR()
                                             .setSwapchains(windowData.swapchain)
                                             .setWaitSemaphores(windowData.renderCompletedSignals[renderSignal])
                                             .setImageIndices(imageIndex));
    if (result != vk::Result::eSuccess) std::cerr << "[TGA Vulkan] Warning: Window Surface has become suboptimal\n";
    // auto& handle = wsi.getWindow(window);
    // auto current = handle.currentFrameIndex;
    // auto cmdBuffer = beginOneTimeCmdBuffer(cmdPool);
    // transitionImageLayout(cmdBuffer, handle.images[current], vk::ImageLayout::eColorAttachmentOptimal,
    //                       vk::ImageLayout::ePresentSrcKHR);
    // cmdBuffer.end();
    // vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    // renderQueue.submit(
    //     {{1, &handle.imageAvailableSemaphore, waitStages, 1, &cmdBuffer, 1, &handle.renderFinishedSemaphore}}, {});
    // wsi.presentImage(window, renderQueue);
    // renderQueue.waitIdle();
    // device.freeCommandBuffers(cmdPool, 1, &cmdBuffer);
    // cmdBuffer = beginOneTimeCmdBuffer(cmdPool);
    // transitionImageLayout(cmdBuffer, handle.images[current], vk::ImageLayout::ePresentSrcKHR,
    //                       vk::ImageLayout::eColorAttachmentOptimal);
    // endOneTimeCmdBuffer(cmdBuffer, cmdPool, renderQueue);
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
    auto& device = state->device;
    auto& data = state->getData(shader);
    if (!data.module) return;

    device.waitIdle();
    device.destroy(data.module);
    data = {};
}

void Interface::free(StagingBuffer buffer)
{
    auto& device = state->device;
    auto& data = state->getData(buffer);
    if (!data.buffer) return;

    device.waitIdle();
    device.destroy(data.buffer);
    device.unmapMemory(data.memory);
    device.free(data.memory);
    data = {};
}

void Interface::free(Buffer buffer)
{
    auto& device = state->device;
    auto& data = state->getData(buffer);
    if (!data.buffer) return;

    device.waitIdle();
    device.destroy(data.buffer);
    device.free(data.memory);
    data = {};
}
void Interface::free(Texture texture)
{
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
    data = {};
}
void Interface::free(Window window)
{
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
    auto& device = state->device;
    auto& data = state->getData(inputSet);
    if (!data.descriptorPool) return;

    device.waitIdle();
    device.destroy(data.descriptorPool);
    data = {};
}
void Interface::free(RenderPass renderPass)
{
    auto& device = state->device;
    auto& data = state->getData(renderPass);
    if (!data.pipeline) return;

    device.waitIdle();
    for (auto& fb : data.framebuffers) device.destroy(fb);
    device.destroy(data.renderPass);

    for (auto& sl : data.layout.setLayouts) device.destroy(sl);
    device.destroy(data.pipeline);
    device.destroy(data.layout.pipelineLayout);
    data = {};
}

void Interface::free(ComputePass computePass)
{
    auto& device = state->device;
    auto& data = state->getData(computePass);
    if (!data.pipeline) return;

    device.waitIdle();
    for (auto& sl : data.layout.setLayouts) device.destroy(sl);
    device.destroy(data.pipeline);
    device.destroy(data.layout.pipelineLayout);
    data = {};
}

void Interface::free(CommandBuffer commandBuffer)
{
    auto& device = state->device;
    auto& cmdPool = state->cmdPool;
    auto& data = state->getData(commandBuffer);
    if (!data.cmdBuffer) return;
    device.waitIdle();
    device.freeCommandBuffers(cmdPool, {data.cmdBuffer});
    device.destroy(data.completionFence);
    data = {};
}

// vkData::Buffer allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
//                                                         vk::MemoryPropertyFlags properties)
// {
//     // TODO use VMA
//     vk::Buffer buffer = device.createBuffer({{}, size, usage, vk::SharingMode::eExclusive, 1, &renderQueueFamily});
//     auto mr = device.getBufferMemoryRequirements(buffer);
//     vk::DeviceMemory memory = device.allocateMemory({mr.size, findMemoryType(mr.memoryTypeBits, properties)});
//     device.bindBufferMemory(buffer, memory, 0);
//     return {buffer, memory, usage, size};
// }

// vk::Format findDepthFormat()
// {
//     std::array<vk::Format, 3> candidates{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
//                                          vk::Format::eD24UnormS8Uint};
//     vk::FormatFeatureFlags features = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
//     for (auto format : candidates) {
//         auto props = pDevice.getFormatProperties(format);
//         if ((props.optimalTilingFeatures & features) == features) return format;
//     }
//     throw std::runtime_error("[TGA Vulkan] Required Depth Format not present on this system");
// }

// vkData::DepthBuffer createDepthBuffer(uint32_t width, uint32_t height)
// {
//     static vk::Format depthFormat = findDepthFormat();
//     vk::Image image = device.createImage({{},
//                                           vk::ImageType::e2D,
//                                           depthFormat,
//                                           {width, height, 1},
//                                           1,
//                                           1,
//                                           vk::SampleCountFlagBits::e1,
//                                           vk::ImageTiling::eOptimal,
//                                           vk::ImageUsageFlagBits::eDepthStencilAttachment,
//                                           vk::SharingMode::eExclusive});
//     auto mr = device.getImageMemoryRequirements(image);
//     vk::DeviceMemory memory =
//         device.allocateMemory({mr.size, findMemoryType(mr.memoryTypeBits,
//         vk::MemoryPropertyFlagBits::eDeviceLocal)});
//     device.bindImageMemory(image, memory, 0);
//     vk::ImageView view = device.createImageView(
//         {{}, image, vk::ImageViewType::e2D, depthFormat, {}, {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}});
//     auto transitionCmdBuffer = beginOneTimeCmdBuffer(cmdPool);
//     transitionImageLayout(transitionCmdBuffer, image, vk::ImageLayout::eUndefined,
//                           vk::ImageLayout::eDepthStencilAttachmentOptimal);
//     endOneTimeCmdBuffer(transitionCmdBuffer, cmdPool, renderQueue);
//     return {image, view, memory};
// }

// vk::RenderPass makeRenderPass(std::vector<vk::Format> const& colorFormats, ClearOperation clearOps,
//                               vk::ImageLayout layout)
// {
//     auto colorLoadOp = vk::AttachmentLoadOp::eLoad;
//     auto depthLoadOp = vk::AttachmentLoadOp::eLoad;
//     if (clearOps == ClearOperation::all || clearOps == ClearOperation::color)
//         colorLoadOp = vk::AttachmentLoadOp::eClear;
//     if (clearOps == ClearOperation::all || clearOps == ClearOperation::depth)
//         depthLoadOp = vk::AttachmentLoadOp::eClear;

//     std::vector<vk::AttachmentDescription> attachments;
//     attachments.reserve(colorFormats.size());

//     for (auto colorFormat : colorFormats) {
//         vk::AttachmentDescription()
//             .setFormat(colorFormat)
//             .setLoadOp(colorLoadOp)
//             .setStoreOp(vk::AttachmentStoreOp::eStore)
//             .setInitialLayout(layout)
//             .setFinalLayout(layout);
//         attachments.push_back({{},
//                                colorFormat,
//                                vk::SampleCountFlagBits::e1,
//                                colorLoadOp,
//                                vk::AttachmentStoreOp::eStore,
//                                vk::AttachmentLoadOp::eDontCare,
//                                vk::AttachmentStoreOp::eDontCare,
//                                layout,
//                                layout});
//     }
//     attachments.push_back({{},
//                            findDepthFormat(),
//                            vk::SampleCountFlagBits::e1,
//                            depthLoadOp,
//                            vk::AttachmentStoreOp::eStore,
//                            vk::AttachmentLoadOp::eDontCare,
//                            vk::AttachmentStoreOp::eDontCare,
//                            vk::ImageLayout::eDepthStencilAttachmentOptimal,
//                            vk::ImageLayout::eDepthStencilAttachmentOptimal});

//     std::vector<vk::AttachmentReference> colorAttachmentRefs;
//     colorAttachmentRefs.reserve(colorFormats.size());
//     for (size_t i = 0; i < colorFormats.size(); ++i)
//         colorAttachmentRefs.push_back({static_cast<uint32_t>(i), vk::ImageLayout::eColorAttachmentOptimal});
//     vk::AttachmentReference depthAttachmentRef{static_cast<uint32_t>(colorAttachmentRefs.size()),
//                                                vk::ImageLayout::eDepthStencilAttachmentOptimal};
//     vk::SubpassDescription subpass{{},
//                                    vk::PipelineBindPoint::eGraphics,
//                                    0,
//                                    0,
//                                    static_cast<uint32_t>(colorAttachmentRefs.size()),
//                                    colorAttachmentRefs.data(),
//                                    nullptr,
//                                    &depthAttachmentRef};

//     vk::PipelineStageFlags pipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
//     vk::SubpassDependency subDependency{
//         VK_SUBPASS_EXTERNAL, 0,  pipelineStageFlags,
//         pipelineStageFlags,  {}, vk::AccessFlagBits::eColorAttachmentRead |
//         vk::AccessFlagBits::eColorAttachmentWrite};
//     return device.createRenderPass(
//         {{}, uint32_t(attachments.size()), attachments.data(), 1, &subpass, 1, &subDependency});
// }

// std::vector<vk::DescriptorSetLayout> decodeInputLayout(const InputLayout& inputLayout)
// {
//     std::vector<vk::DescriptorSetLayout> descSetLayouts{};
//     for (const auto& setLayout : inputLayout.setLayouts) {
//         std::vector<vk::DescriptorSetLayoutBinding> bindings{};
//         for (uint32_t i = 0; i < setLayout.bindingLayouts.size(); i++) {
//             bindings.emplace_back(
//                 vk::DescriptorSetLayoutBinding{i, determineDescriptorType(setLayout.bindingLayouts[i].type),
//                                                setLayout.bindingLayouts[i].count, vk::ShaderStageFlagBits::eAll});
//         }
//         descSetLayouts.emplace_back(device.createDescriptorSetLayout({{}, uint32_t(bindings.size()),
//         bindings.data()}));
//     }
//     return descSetLayouts;
// }

// std::pair<vk::Pipeline, vk::PipelineBindPoint> makePipeline(const RenderPassInfo& renderPassInfo,
//                                                             vk::PipelineLayout pipelineLayout,
//                                                             vk::RenderPass renderPass)
// {
//     bool isValid = renderPassInfo.shaderStages.size() > 0;
//     bool vertexPresent{false};
//     bool fragmentPresent{false};
//     for (auto stage : renderPassInfo.shaderStages) {
//         const auto& shader = shaders[stage];
//         if (shader.type == ShaderType::compute) {
//             if (renderPassInfo.shaderStages.size() == 1) {
//                 return {
//                     device
//                         .createComputePipeline(
//                             {}, {{}, {{}, vk::ShaderStageFlagBits::eCompute, shader.module, "main"}, pipelineLayout})
//                         .value,
//                     vk::PipelineBindPoint::eCompute};
//             } else {
//                 isValid = false;
//                 break;
//             }
//         } else if (shader.type == ShaderType::vertex) {
//             isValid = isValid && !fragmentPresent && !vertexPresent;
//             vertexPresent = true;
//         } else if (shader.type == ShaderType::fragment) {
//             isValid = isValid && !fragmentPresent && vertexPresent;
//             fragmentPresent = true;
//         } else {
//             isValid = false;
//             break;
//         }
//     }
//     if (!isValid) throw std::runtime_error("[TGA Vulkan] Invalid Shader Stage Configuration");
//     return {makeGraphicsPipeline(renderPassInfo, pipelineLayout, renderPass), vk::PipelineBindPoint::eGraphics};
// }

// vk::CommandBuffer beginOneTimeCmdBuffer(vk::CommandPool& cmdPool)
// {
//     vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers({cmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
//     cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
//     return cmdBuffer;
// }
// void endOneTimeCmdBuffer(vk::CommandBuffer& cmdBuffer, vk::CommandPool& cmdPool, vk::Queue& submitQueue)
// {
//     cmdBuffer.end();
//     submitQueue.submit({{0, nullptr, nullptr, 1, &cmdBuffer}}, {});
//     submitQueue.waitIdle();
//     device.freeCommandBuffers(cmdPool, 1, &cmdBuffer);
// }

// void fillBuffer(size_t size, const uint8_t *data, uint32_t offset, vk::Buffer target)
// {
//     auto copyCmdBuffer = beginOneTimeCmdBuffer(transferCmdPool);
//     if (size <= 65536 && (size % 4) == 0)  // Quick Path
//     {
//         copyCmdBuffer.updateBuffer(target, offset, size, data);
//         endOneTimeCmdBuffer(copyCmdBuffer, transferCmdPool, renderQueue);
//     } else  // Staging Buffer
//     {
//         auto buffer =
//             allocateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
//                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//         auto mapping = device.mapMemory(buffer.memory, 0, size, {});
//         std::memcpy(mapping, data, size);
//         device.unmapMemory(buffer.memory);
//         vk::BufferCopy region{0, offset, size};
//         copyCmdBuffer.copyBuffer(buffer.buffer, target, {region});
//         endOneTimeCmdBuffer(copyCmdBuffer, transferCmdPool, renderQueue);
//         device.destroy(buffer.buffer);
//         device.free(buffer.memory);
//     }
// }

// void fillTexture(size_t size, const uint8_t *data, vk::Extent3D extent, uint32_t layers, vk::Image target)
// {
//     auto buffer = allocateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
//                                  vk::MemoryPropertyFlagBits::eHostVisible |
//                                  vk::MemoryPropertyFlagBits::eHostCoherent);
//     auto mapping = device.mapMemory(buffer.memory, 0, size, {});
//     std::memcpy(mapping, data, size);
//     device.unmapMemory(buffer.memory);

//     auto uploadCmd = device.allocateCommandBuffers({cmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
//     uploadCmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
//     uploadCmd.copyBufferToImage(buffer.buffer, target, vk::ImageLayout::eTransferDstOptimal,
//                                 vk::BufferImageCopy()
//                                     .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layers})
//                                     .setImageExtent(extent));

//     uploadCmd.end();
//     auto cmdCompleteSignal = device.createSemaphore({});
//     renderQueue.submit(vk::SubmitInfo().setCommandBuffers(uploadCmd).setSignalSemaphores(cmdCompleteSignal));
//     device.waitSemaphores(vk::SemaphoreWaitInfo().setSemaphores(cmdCompleteSignal),
//                           std::numeric_limits<uint64_t>::max());
//     device.freeCommandBuffers(cmdPool, 1, &uploadCmd);
//     device.destroySemaphore(cmdCompleteSignal);

//     device.destroy(buffer.buffer);
//     device.free(buffer.memory);
// }

}  // namespace tga