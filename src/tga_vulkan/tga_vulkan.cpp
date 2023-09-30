#include "tga/tga_vulkan/tga_vulkan.hpp"

#include "tga/tga_vulkan/tga_vulkan_debug.hpp"
#include "tga/tga_vulkan/tga_vulkan_extensions.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                          VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void* pUserData)
{
    (void)messageSeverity;  // Warnings Silencer
    (void)messageType;      // Warnings Silencer
    (void)pUserData;        // Warning Silencer
    std::cerr << "[VULKAN VALIDATION LAYER]: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

namespace tga
{
namespace /*conversion*/
{
    vk::BufferUsageFlags determineBufferFlags(tga::BufferUsage usage)
    {
        if (usage == BufferUsage::undefined) throw std::runtime_error("[TGA Vulkan] Buffer usage is undefined!");
        vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc;

        auto tgaToVkFlag = [&](tga::BufferUsage tgaFlag, vk::BufferUsageFlagBits vkFlag) {
            if (usage & tgaFlag) usageFlags |= vkFlag;
        };

        tgaToVkFlag(tga::BufferUsage::uniform, vk::BufferUsageFlagBits::eUniformBuffer);
        tgaToVkFlag(tga::BufferUsage::vertex, vk::BufferUsageFlagBits::eVertexBuffer);
        tgaToVkFlag(tga::BufferUsage::index, vk::BufferUsageFlagBits::eIndexBuffer);
        tgaToVkFlag(tga::BufferUsage::storage, vk::BufferUsageFlagBits::eStorageBuffer);
        tgaToVkFlag(tga::BufferUsage::indirect, vk::BufferUsageFlagBits::eIndirectBuffer);
        tgaToVkFlag(tga::BufferUsage::accelerationStructureBuildInput,
                    vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

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

    std::tuple<vk::Filter, vk::SamplerAddressMode> determineSamplerInfo(const TextureInfo& textureInfo)
    {
        auto filter = vk::Filter::eNearest;
        if (textureInfo.samplerMode == SamplerMode::linear) filter = vk::Filter::eLinear;
        vk::SamplerAddressMode addressMode{vk::SamplerAddressMode::eClampToBorder};
        switch (textureInfo.addressMode) {
            case AddressMode::clampEdge: addressMode = vk::SamplerAddressMode::eClampToEdge; break;
            case AddressMode::clampBorder: addressMode = vk::SamplerAddressMode::eClampToBorder; break;
            case AddressMode::repeat: addressMode = vk::SamplerAddressMode::eRepeat; break;
            case AddressMode::repeatMirror: addressMode = vk::SamplerAddressMode::eMirroredRepeat; break;
        }
        return {filter, addressMode};
    }

    vk::ShaderStageFlagBits determineShaderStage(tga::ShaderType shaderType)
    {
        switch (shaderType) {
            case ShaderType::vertex: return vk::ShaderStageFlagBits::eVertex;
            case ShaderType::fragment: return vk::ShaderStageFlagBits::eFragment;
            case ShaderType::compute: return vk::ShaderStageFlagBits::eCompute;
            default: return vk::ShaderStageFlagBits::eAllGraphics;
        }
    }

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

    vk::DescriptorType determineDescriptorType(tga::BindingType bindingType)
    {
        switch (bindingType) {
            case BindingType::uniformBuffer: return vk::DescriptorType::eUniformBuffer;
            case BindingType::sampler: return vk::DescriptorType::eCombinedImageSampler;
            case BindingType::storageBuffer: return vk::DescriptorType::eStorageBuffer;
            default: return vk::DescriptorType::eInputAttachment;
        }
    }

    vk::AccessFlags layoutToAccessFlags(vk::ImageLayout layout)
    {
        switch (layout) {
            case vk::ImageLayout::eUndefined: return {};
            case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
            case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits::eTransferRead;
            case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
            case vk::ImageLayout::eColorAttachmentOptimal:
                return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
            case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits::eMemoryRead;
            case vk::ImageLayout::eGeneral: return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                return vk::AccessFlagBits::eDepthStencilAttachmentRead |
                       vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            case vk::ImageLayout::eDepthAttachmentOptimal:
                return vk::AccessFlagBits::eDepthStencilAttachmentRead |
                       vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            case vk::ImageLayout::eStencilAttachmentOptimal:
                return vk::AccessFlagBits::eDepthStencilAttachmentRead |
                       vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            default: throw std::runtime_error("[TGA Vulkan] Layout to AccessFlags transition not supported"); ;
        }
    }

    // TODO use?
    vk::PipelineStageFlags layoutToPipelineStageFlags(vk::ImageLayout layout)
    {
        switch (layout) {
            case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
            case vk::ImageLayout::eTransferDstOptimal: return vk::PipelineStageFlagBits::eTransfer;
            case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits::eTransfer;
            case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eVertexShader;
            case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
            case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits::eAllGraphics;
            case vk::ImageLayout::eGeneral: return vk::PipelineStageFlagBits::eAllCommands;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
            case vk::ImageLayout::eDepthAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
            case vk::ImageLayout::eStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
            default: throw std::runtime_error("[TGA Vulkan] Layout to PipelineStageFlags transition not supported");
        }
    }

    vk::PipelineStageFlags accessToPipelineStageFlags(vk::AccessFlags accessFlags)
    {
        if (accessFlags == vk::AccessFlags{}) return vk::PipelineStageFlagBits::eTopOfPipe;
        vk::PipelineStageFlags pipelineStageFlags{};
        if ((accessFlags & vk::AccessFlagBits::eIndirectCommandRead) == vk::AccessFlagBits::eIndirectCommandRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eDrawIndirect;
        if ((accessFlags & vk::AccessFlagBits::eIndexRead) == vk::AccessFlagBits::eIndexRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexInput;
        if ((accessFlags & vk::AccessFlagBits::eVertexAttributeRead) == vk::AccessFlagBits::eVertexAttributeRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexInput;
        if ((accessFlags & vk::AccessFlagBits::eUniformRead) == vk::AccessFlagBits::eUniformRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexShader;
        if ((accessFlags & vk::AccessFlagBits::eShaderRead) == vk::AccessFlagBits::eShaderRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexShader;
        if ((accessFlags & vk::AccessFlagBits::eShaderWrite) == vk::AccessFlagBits::eShaderWrite)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eVertexShader;
        if ((accessFlags & vk::AccessFlagBits::eInputAttachmentRead) == vk::AccessFlagBits::eInputAttachmentRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eFragmentShader;
        if ((accessFlags & vk::AccessFlagBits::eColorAttachmentRead) == vk::AccessFlagBits::eColorAttachmentRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
        if ((accessFlags & vk::AccessFlagBits::eColorAttachmentWrite) == vk::AccessFlagBits::eColorAttachmentWrite)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
        if ((accessFlags & vk::AccessFlagBits::eDepthStencilAttachmentRead) ==
            vk::AccessFlagBits::eDepthStencilAttachmentRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        if ((accessFlags & vk::AccessFlagBits::eDepthStencilAttachmentWrite) ==
            vk::AccessFlagBits::eDepthStencilAttachmentWrite)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
        if ((accessFlags & vk::AccessFlagBits::eTransferRead) == vk::AccessFlagBits::eTransferRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eTransfer;
        if ((accessFlags & vk::AccessFlagBits::eTransferWrite) == vk::AccessFlagBits::eTransferWrite)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eTransfer;
        if ((accessFlags & vk::AccessFlagBits::eHostRead) == vk::AccessFlagBits::eHostRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eHost;
        if ((accessFlags & vk::AccessFlagBits::eHostWrite) == vk::AccessFlagBits::eHostWrite)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eHost;
        if ((accessFlags & vk::AccessFlagBits::eMemoryRead) == vk::AccessFlagBits::eMemoryRead)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eBottomOfPipe;
        if ((accessFlags & vk::AccessFlagBits::eMemoryWrite) == vk::AccessFlagBits::eMemoryWrite)
            pipelineStageFlags |= vk::PipelineStageFlagBits::eAllGraphics;
        return pipelineStageFlags;
    }

}  // namespace

namespace /*init*/
{
    static const std::array<const char*, 1> vulkanLayers = {"VK_LAYER_KHRONOS_validation"};
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
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) gpus = p;
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

        auto extensions = [](bool withRayQuerySupport) -> std::vector<const char*> {
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
    void transitionImageLayout(vk::CommandBuffer cmdBuffer, vk::Image image, vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout, uint32_t renderQueueFamiliy)
    {
        vk::ImageAspectFlags imageAspects{};
        if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal ||
            newLayout == vk::ImageLayout::eDepthAttachmentOptimal ||
            newLayout == vk::ImageLayout::eStencilAttachmentOptimal) {
            imageAspects = vk::ImageAspectFlagBits::eDepth;
            imageAspects |= vk::ImageAspectFlagBits::eStencil;
        } else {
            imageAspects = vk::ImageAspectFlagBits::eColor;
        }

        auto accessFlagsOld = layoutToAccessFlags(oldLayout);
        auto accessFlagsNew = layoutToAccessFlags(newLayout);
        cmdBuffer.pipelineBarrier(accessToPipelineStageFlags(accessFlagsOld),
                                  accessToPipelineStageFlags(accessFlagsNew), {}, {}, {},
                                  {{accessFlagsOld,
                                    accessFlagsNew,
                                    oldLayout,
                                    newLayout,
                                    renderQueueFamiliy,
                                    renderQueueFamiliy,
                                    image,
                                    {imageAspects, 0, 1, 0, VK_REMAINING_ARRAY_LAYERS}}});
    }

    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& mProps, uint32_t typeFilter,
                            vk::MemoryPropertyFlags properties)
    {
        for (uint32_t i = 0; i < mProps.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && ((mProps.memoryTypes[i].propertyFlags & properties) == properties)) return i;
        }
        throw std::runtime_error("[TGA Vulkan] Memory Type could not be found");
    }

}  // namespace

Interface::InternalState::InternalState()
    : wsi(VulkanWSI()), instance(createInstance(wsi)), debugger(createDebugMessenger(instance)),
      pDevice(choseGPU(instance)), renderQueueFamiliy(findUniversalQueueFamily(pDevice)),
      device(createDevice(pDevice, renderQueueFamiliy)), renderQueue(device.getQueue(renderQueueFamiliy, 0)),
      transferCmdPool(device.createCommandPool({{}, renderQueueFamiliy})),
      graphicsCmdPool(
          device.createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer, renderQueueFamiliy}))
{}

Interface::Interface() : state(std::make_unique<InternalState>())
{
    std::cout << "TGA Vulkan Created\n";
    createBottomLevelAccelerationStructure({});
}

Interface::InternalState::~InternalState()
{
    device.waitIdle();
    while (shaders.size() > 0) free(shaders.begin()->first);
    while (buffers.size() > 0) free(buffers.begin()->first);
    while (textures.size() > 0) free(textures.begin()->first);
    while (wsi.windows.size() > 0) free(wsi.windows.begin()->first);
    while (inputSets.size() > 0) free(inputSets.begin()->first);
    while (renderPasses.size() > 0) free(renderPasses.begin()->first);
    device.destroy(transferCmdPool);
    device.destroy(graphicsCmdPool);
    device.destroy();
    if (debugger) instance.destroy(debugger);
    instance.destroy();
}

/*Interface Methodes*/
Shader Interface::createShader(const ShaderInfo& shaderInfo)
{
    auto& device = state->device;
    auto& shaders = state->shaders;

    vk::ShaderModule module =
        device.createShaderModule({{}, shaderInfo.srcSize, reinterpret_cast<const uint32_t*>(shaderInfo.src)});
    Shader handle = Shader(TgaShader(VkShaderModule(module)));
    Shader_vkData shader{module, shaderInfo.type};
    shaders.emplace(handle, shader);
    return handle;
}
Buffer Interface::createBuffer(const BufferInfo& bufferInfo)
{
    auto& buffers = state->buffers;

    auto usage = determineBufferFlags(bufferInfo.usage);
    Buffer_vkData buffer = state->allocateBuffer(bufferInfo.dataSize, usage, vk::MemoryPropertyFlagBits::eDeviceLocal);
    Buffer handle = Buffer(TgaBuffer(VkBuffer(buffer.buffer)));
    buffers.emplace(handle, buffer);
    if (bufferInfo.data != nullptr) state->fillBuffer(bufferInfo.dataSize, bufferInfo.data, 0, buffer.buffer);
    return handle;
}
Texture Interface::createTexture(const TextureInfo& textureInfo)
{
    auto& device = state->device;
    auto& pDevice = state->pDevice;
    auto& textures = state->textures;
    auto& graphicsCmdPool = state->graphicsCmdPool;
    auto& renderQueueFamiliy = state->renderQueueFamiliy;
    auto& renderQueue = state->renderQueue;

    vk::Format format = determineImageFormat(textureInfo.format);
    auto [extent, layers] = [](tga::TextureInfo const& textureInfo) -> std::pair<vk::Extent3D, uint32_t> {
        uint32_t depth = 1;
        uint32_t layers = 1;
        if (textureInfo.textureType == TextureType::_3D)
            depth = textureInfo.depthLayers;
        else if (textureInfo.textureType != TextureType::_2D)
            layers = textureInfo.depthLayers;

        return {{textureInfo.width, textureInfo.height, depth}, layers};
    }(textureInfo);
    auto [imageType, imageViewType, flags] =
        [](const TextureInfo& textureInfo) -> std::tuple<vk::ImageType, vk::ImageViewType, vk::ImageCreateFlags> {
        switch (textureInfo.textureType) {
            case TextureType::_2D: return {vk::ImageType::e2D, vk::ImageViewType::e2D, {}};
            case TextureType::_2DArray: return {vk::ImageType::e2D, vk::ImageViewType::e2DArray, {}};
            case TextureType::_3D: return {vk::ImageType::e3D, vk::ImageViewType::e3D, {}};
            case TextureType::_Cube:
                return {vk::ImageType::e2D, vk::ImageViewType::eCube, vk::ImageCreateFlagBits::eCubeCompatible};
            default: return {vk::ImageType::e2D, vk::ImageViewType::e2D, {}};
        }
    }(textureInfo);

    auto [tiling, usageFlags] = [](vk::PhysicalDevice& pDevice,
                                   vk::Format& format) -> std::pair<vk::ImageTiling, vk::ImageUsageFlags> {
        auto tiling = vk::ImageTiling::eOptimal;
        auto usageFlags = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst |
                          vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eColorAttachment;
        auto formatProps = pDevice.getFormatProperties(format);
        if (!(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage))
            throw std::runtime_error("[TGA Vulkan] Chosen Image Format: " + vk::to_string(format) +
                                     " cannot be used as Texture on this System");
        if (!(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eTransferDst))
            throw std::runtime_error("[TGA Vulkan] Chosen Image Format: " + vk::to_string(format) +
                                     " cannot be written to on this System");
        if (!(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) ||
            !(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eTransferSrc)) {
            std::cerr << "[TGA Vulkan] Warning: Chosen Image Format: " << vk::to_string(format)
                      << " cannot be used as a Framebuffer on this System\n";
            usageFlags = usageFlags & (~vk::ImageUsageFlagBits::eColorAttachment);  // Can't write to it
            usageFlags = usageFlags & (~vk::ImageUsageFlagBits::eTransferSrc);      // No write means no read necessary
        }
        return {tiling, usageFlags};
    }(pDevice, format);

    vk::Image image = device.createImage({flags, imageType, format, extent, 1, layers, vk::SampleCountFlagBits::e1,
                                          tiling, usageFlags, vk::SharingMode::eExclusive});
    auto mr = device.getImageMemoryRequirements(image);
    vk::DeviceMemory memory =
        device.allocateMemory(vk::MemoryAllocateInfo().setAllocationSize(mr.size).setMemoryTypeIndex(
            findMemoryType(pDevice.getMemoryProperties(), mr.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)));
    device.bindImageMemory(image, memory, 0);
    vk::ImageView view = device.createImageView(
        {{}, image, imageViewType, format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, layers}});

    auto [filter, addressMode] = determineSamplerInfo(textureInfo);
    vk::Sampler sampler = device.createSampler(
        {{}, filter, filter, vk::SamplerMipmapMode::eLinear, addressMode, addressMode, addressMode});
    Texture_vkData texture{image, view, memory, sampler, extent, format};
    Texture handle = Texture(TgaTexture(VkImage(image)));
    textures.emplace(handle, texture);

    auto transitionCmdBuffer = device.allocateCommandBuffers({graphicsCmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
    transitionCmdBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    auto textureCreatedSignal = device.createSemaphore({});

    if (!textureInfo.data) {
        transitionImageLayout(transitionCmdBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
                              renderQueueFamiliy);
        transitionCmdBuffer.end();
        renderQueue.submit(
            vk::SubmitInfo().setCommandBuffers(transitionCmdBuffer).setSignalSemaphores(textureCreatedSignal));
        std::ignore = device.waitSemaphores(vk::SemaphoreWaitInfo().setSemaphores(textureCreatedSignal),
                                            std::numeric_limits<uint64_t>::max());
        device.freeCommandBuffers(graphicsCmdPool, 1, &transitionCmdBuffer);
        device.destroySemaphore(textureCreatedSignal);
        return handle;
    }

    transitionImageLayout(transitionCmdBuffer, image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                          renderQueueFamiliy);
    // endOneTimeCmdBuffer(transitionCmdBuffer, graphicsCmdPool, renderQueue);

    vk::Buffer buffer = device.createBuffer(vk::BufferCreateInfo()
                                                .setSize(textureInfo.dataSize)
                                                .setUsage(vk::BufferUsageFlagBits::eTransferSrc)
                                                .setSharingMode(vk::SharingMode::eExclusive)
                                                .setQueueFamilyIndices(renderQueueFamiliy));
    vk::DeviceMemory memory = device.allocateMemory(
        {device.getBufferMemoryRequirements(buffer).size, findMemoryType(mr.memoryTypeBits, properties)});
    device.bindBufferMemory(buffer, memory, 0);
    auto buffer = allocateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    auto mapping = device.mapMemory(buffer.memory, 0, size, {});
    std::memcpy(mapping, data, size);
    device.unmapMemory(buffer.memory);

    auto uploadCmd = device.allocateCommandBuffers({graphicsCmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
    uploadCmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    uploadCmd.copyBufferToImage(buffer.buffer, target, vk::ImageLayout::eTransferDstOptimal,
                                vk::BufferImageCopy()
                                    .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layers})
                                    .setImageExtent(extent));

    uploadCmd.end();
    auto cmdCompleteSignal = device.createSemaphore({});
    renderQueue.submit(vk::SubmitInfo().setCommandBuffers(uploadCmd).setSignalSemaphores(cmdCompleteSignal));
    std::ignore = device.waitSemaphores(vk::SemaphoreWaitInfo().setSemaphores(cmdCompleteSignal),
                                        std::numeric_limits<uint64_t>::max());
    device.freeCommandBuffers(graphicsCmdPool, 1, &uploadCmd);
    device.destroySemaphore(cmdCompleteSignal);

    device.destroy(buffer.buffer);
    device.free(buffer.memory);

    // fillTexture(textureInfo.dataSize, textureInfo.data, extent, layers, image);
    transitionCmdBuffer = beginOneTimeCmdBuffer(graphicsCmdPool);
    transitionImageLayout(transitionCmdBuffer, image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral);
    endOneTimeCmdBuffer(transitionCmdBuffer, graphicsCmdPool, renderQueue);

    return handle;
}
Window Interface::createWindow(const WindowInfo& windowInfo)
{
    auto window = wsi.createWindow(windowInfo, instance, pDevice, device, renderQueueFamiliy);
    auto& handle = wsi.getWindow(window);
    auto transitionCmdBuffer = beginOneTimeCmdBuffer(graphicsCmdPool);
    for (auto& image : handle.images)
        transitionImageLayout(transitionCmdBuffer, image, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eColorAttachmentOptimal);
    endOneTimeCmdBuffer(transitionCmdBuffer, graphicsCmdPool, renderQueue);
    return window;
}
InputSet Interface::createInputSet(const InputSetInfo& inputSetInfo)
{
    if (renderPasses[inputSetInfo.targetRenderPass].setLayouts.size() <= inputSetInfo.setIndex)
        throw std::runtime_error("[TGA Vulkan] InputSet does not match layout from RenderPass");

    uint32_t uniformCount = 0;
    uint32_t storageCount = 0;
    uint32_t textureCount = 0;
    for (auto& binding : inputSetInfo.bindings) {
        if (auto handle = std::get_if<Buffer>(&binding.resource)) {
            const auto& buffer = buffers[*handle];
            if (buffer.flags & vk::BufferUsageFlagBits::eUniformBuffer) uniformCount++;
            if (buffer.flags & vk::BufferUsageFlagBits::eStorageBuffer) storageCount++;
        } else
            textureCount++;
    }
    std::vector<vk::DescriptorPoolSize> poolSizes{};
    if (uniformCount > 0)
        poolSizes.emplace_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, uniformCount));
    if (storageCount > 0)
        poolSizes.emplace_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, storageCount));
    if (textureCount > 0)
        poolSizes.emplace_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, textureCount));

    vk::DescriptorPool descPool = device.createDescriptorPool({{}, 1, uint32_t(poolSizes.size()), poolSizes.data()});

    auto layout = renderPasses[inputSetInfo.targetRenderPass].setLayouts[inputSetInfo.setIndex];
    vk::DescriptorSet descSet = device.allocateDescriptorSets({descPool, 1, &layout})[0];
    for (auto& binding : inputSetInfo.bindings) {
        if (auto resource = std::get_if<Buffer>(&binding.resource)) {
            auto& buffer = buffers[*resource];
            vk::DescriptorBufferInfo bufferInfo{buffer.buffer, 0, VK_WHOLE_SIZE};
            vk::WriteDescriptorSet writeSet{
                descSet,
                binding.slot,
                binding.arrayElement,
                1,
                (buffer.flags & vk::BufferUsageFlagBits::eStorageBuffer ? vk::DescriptorType::eStorageBuffer
                                                                        : vk::DescriptorType::eUniformBuffer),
                {},
                &bufferInfo};
            device.updateDescriptorSets({writeSet}, {});
        } else if (auto resource = std::get_if<Texture>(&binding.resource)) {
            auto& texture = textures[*resource];
            vk::DescriptorImageInfo imageInfo{texture.sampler, texture.imageView, vk::ImageLayout::eGeneral};
            vk::WriteDescriptorSet writeSet{
                descSet, binding.slot, binding.arrayElement, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo};
            device.updateDescriptorSets({writeSet}, {});
        }
    }
    InputSet inputSet = InputSet(TgaInputSet(VkDescriptorPool(descPool)));
    InputSet_vkData inputSet_tv{descPool, descSet, inputSetInfo.setIndex};
    inputSets.emplace(inputSet, inputSet_tv);
    return inputSet;
}

// Todo: Separate ComputePass
RenderPass Interface::createRenderPass(const RenderPassInfo& renderPassInfo)
{
    vk::RenderPass renderPass;
    std::vector<vk::Framebuffer> framebuffers;
    vk::Extent2D area{};
    if (auto renderTarget = std::get_if<Texture>(&renderPassInfo.renderTarget)) {
        auto& renderTex = textures[*renderTarget];
        area = vk::Extent2D(renderTex.extent.width, renderTex.extent.height);
        if (!textureDepthBuffers.count(*renderTarget))
            textureDepthBuffers.emplace(*renderTarget, createDepthBuffer(area.width, area.height));
        auto& depthBuffer = textureDepthBuffers[*renderTarget];
        renderPass = makeRenderPass({renderTex.format}, renderPassInfo.clearOperations, vk::ImageLayout::eGeneral);
        std::array<vk::ImageView, 2> attachments{renderTex.imageView, depthBuffer.imageView};
        framebuffers.emplace_back(device.createFramebuffer({{},
                                                            renderPass,
                                                            static_cast<uint32_t>(attachments.size()),
                                                            attachments.data(),
                                                            area.width,
                                                            area.height,
                                                            1}));
    } else if (auto renderTargets = std::get_if<std::vector<Texture>>(&renderPassInfo.renderTarget)) {
        assert(!renderTargets->empty());

        vk::ImageView depthBufferImageView{};
        std::vector<vk::Format> renderTexFormats;
        renderTexFormats.reserve(renderTargets->size());
        std::vector<vk::ImageView> attachments;
        attachments.reserve(renderTargets->size() + 1);
        for (auto renderTarget : *renderTargets) {
            auto& renderTex = textures.at(renderTarget);
            renderTexFormats.push_back(renderTex.format);
            attachments.push_back(renderTex.imageView);
            if (depthBufferImageView != vk::ImageView{}) continue;

            area = vk::Extent2D(renderTex.extent.width, renderTex.extent.height);
            if (!textureDepthBuffers.count(renderTarget))
                textureDepthBuffers.emplace(renderTarget, createDepthBuffer(area.width, area.height));
            depthBufferImageView = textureDepthBuffers[renderTarget].imageView;
        }
        attachments.push_back(depthBufferImageView);

        renderPass = makeRenderPass(renderTexFormats, renderPassInfo.clearOperations, vk::ImageLayout::eGeneral);
        // std::array<vk::ImageView, 2> attachments{renderTex.imageView, depthBuffer.imageView};
        framebuffers.emplace_back(device.createFramebuffer({{},
                                                            renderPass,
                                                            static_cast<uint32_t>(attachments.size()),
                                                            attachments.data(),
                                                            area.width,
                                                            area.height,
                                                            1}));
    } else if (auto renderTarget = std::get_if<Window>(&renderPassInfo.renderTarget)) {
        auto& renderWindow = wsi.getWindow(*renderTarget);
        area = renderWindow.extent;
        if (!windowDepthBuffers.count(*renderTarget))
            windowDepthBuffers.emplace(*renderTarget,
                                       createDepthBuffer(renderWindow.extent.width, renderWindow.extent.height));
        auto& depthBuffer = windowDepthBuffers[*renderTarget];
        renderPass = makeRenderPass({renderWindow.format}, renderPassInfo.clearOperations,
                                    vk::ImageLayout::eColorAttachmentOptimal);
        for (uint32_t i = 0; i < renderWindow.imageViews.size(); i++) {
            std::array<vk::ImageView, 2> attachments{renderWindow.imageViews[i], depthBuffer.imageView};
            framebuffers.emplace_back(device.createFramebuffer({{},
                                                                renderPass,
                                                                static_cast<uint32_t>(attachments.size()),
                                                                attachments.data(),
                                                                renderWindow.extent.width,
                                                                renderWindow.extent.height,
                                                                1}));
        }
    }
    std::vector<vk::DescriptorSetLayout> setLayouts = decodeInputLayout(renderPassInfo.inputLayout);

    auto pipelineLayout = device.createPipelineLayout({{}, uint32_t(setLayouts.size()), setLayouts.data()});
    auto [pipeline, bindPoint] = makePipeline(renderPassInfo, pipelineLayout, renderPass);
    RenderPass_vkData renderPass_tv{framebuffers, renderPass, setLayouts, pipelineLayout, pipeline, bindPoint, area};
    RenderPass handle = RenderPass(TgaRenderPass(VkRenderPass(renderPass)));
    renderPasses.emplace(handle, renderPass_tv);
    return handle;
}

ext::TopLevelAccelerationStructure Interface::InternalState::createTopLevelAccelerationStructure(
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
    if (0) {
        int* ptr;
        int const& ref = *ptr;
    }
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

void Interface::beginCommandBuffer()
{
    if (currentRecording.cmdBuffer) throw std::runtime_error("[TGA Vulkan] Another Commandbuffer is still recording!");
    currentRecording.cmdBuffer =
        device.allocateCommandBuffers({graphicsCmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
    currentRecording.cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
}
void Interface::beginCommandBuffer(CommandBuffer cmdBuffer)
{
    if (currentRecording.cmdBuffer) throw std::runtime_error("[TGA Vulkan] Another Commandbuffer is still recording!");
    if (!cmdBuffer) {
        return beginCommandBuffer();
    }
    auto& handle = commandBuffers[cmdBuffer];
    currentRecording.cmdBuffer = handle.cmdBuffer;
    handle.cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eSimultaneousUse});
}
void Interface::bindVertexBuffer(Buffer buffer)
{
    auto& handle = buffers[buffer];
    currentRecording.cmdBuffer.bindVertexBuffers(0, {handle.buffer}, {0});
}
void Interface::bindIndexBuffer(Buffer buffer)
{
    auto& handle = buffers[buffer];
    currentRecording.cmdBuffer.bindIndexBuffer(handle.buffer, 0, vk::IndexType::eUint32);
}

void Interface::bindInputSet(InputSet inputSet)
{
    auto& handle = inputSets[inputSet];

    auto& renderPass = renderPasses[currentRecording.renderPass];
    currentRecording.cmdBuffer.bindDescriptorSets(renderPass.bindPoint, renderPass.pipelineLayout, handle.index, 1,
                                                  &handle.descriptorSet, 0, nullptr);
}
void Interface::draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount, uint32_t firstInstance)
{
    currentRecording.cmdBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}
void Interface::drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t instanceCount,
                            uint32_t firstInstance)
{
    currentRecording.cmdBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void Interface::drawIndirect(Buffer buffer, uint32_t drawCount, size_t offset, uint32_t stride)
{
    currentRecording.cmdBuffer.drawIndirect(buffers[buffer].buffer, offset, drawCount, stride);
}
void Interface::drawIndexedIndirect(Buffer buffer, uint32_t drawCount, size_t offset, uint32_t stride)
{
    currentRecording.cmdBuffer.drawIndexedIndirect(buffers[buffer].buffer, offset, drawCount, stride);
}
void Interface::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    currentRecording.cmdBuffer.dispatch(groupCountX, groupCountY, groupCountZ);
}

void Interface::setRenderPass(RenderPass renderPass, uint32_t framebufferIndex)
{
    if (currentRecording.renderPass) {
        if (renderPasses[currentRecording.renderPass].bindPoint == vk::PipelineBindPoint::eGraphics)
            currentRecording.cmdBuffer.endRenderPass();
    }
    auto& cmd = currentRecording.cmdBuffer;
    auto& handle = renderPasses[renderPass];
    std::array<float, 4> colorClear = {0., 0., 0., 0.};
    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0] = vk::ClearColorValue(colorClear);
    clearValues[1] = vk::ClearDepthStencilValue(1.f, 0);

    if (handle.bindPoint == vk::PipelineBindPoint::eGraphics) {
        uint32_t frameIndex = std::min(framebufferIndex, uint32_t(handle.framebuffers.size() - 1));
        cmd.beginRenderPass({handle.renderPass,
                             handle.framebuffers[frameIndex],
                             {{}, handle.area},
                             static_cast<uint32_t>(clearValues.size()),
                             clearValues.data()},
                            vk::SubpassContents::eInline);
        cmd.bindPipeline(handle.bindPoint, handle.pipeline);
        cmd.setViewport(0, {{0, 0, float(handle.area.width), float(handle.area.height), 0, 1}});
        cmd.setScissor(0, {{{}, handle.area}});
    } else {
        cmd.bindPipeline(handle.bindPoint, handle.pipeline);
    }

    currentRecording.renderPass = renderPass;
}
CommandBuffer Interface::endCommandBuffer()
{
    if (currentRecording.renderPass) {
        if (renderPasses[currentRecording.renderPass].bindPoint == vk::PipelineBindPoint::eGraphics)
            currentRecording.cmdBuffer.endRenderPass();
        currentRecording.renderPass = RenderPass();
    }
    currentRecording.cmdBuffer.end();
    CommandBuffer_vkData cmdBuffer_tv{currentRecording.cmdBuffer};
    CommandBuffer handle = TgaCommandBuffer(VkCommandBuffer(currentRecording.cmdBuffer));
    commandBuffers.emplace(handle, cmdBuffer_tv);
    currentRecording.cmdBuffer = vk::CommandBuffer();
    return handle;
}
void Interface::execute(CommandBuffer commandBuffer)
{
    auto& handle = commandBuffers[commandBuffer];
    renderQueue.submit({{0, nullptr, nullptr, 1, &handle.cmdBuffer}}, {});
}

void Interface::updateBuffer(Buffer buffer, uint8_t const* data, size_t dataSize, uint32_t offset)
{
    auto& handle = buffers[buffer];
    fillBuffer(dataSize, data, offset, handle.buffer);
}

std::vector<uint8_t> Interface::readback(Buffer buffer)
{
    auto& handle = buffers[buffer];
    std::vector<uint8_t> rbBuffer{};
    rbBuffer.resize(handle.size);

    auto staging = allocateBuffer(handle.size, vk::BufferUsageFlagBits::eTransferDst,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    auto copyCmdBuffer = beginOneTimeCmdBuffer(transferCmdPool);
    vk::BufferCopy region{0, 0, handle.size};
    copyCmdBuffer.copyBuffer(handle.buffer, staging.buffer, {region});
    endOneTimeCmdBuffer(copyCmdBuffer, transferCmdPool, renderQueue);
    auto mapping = device.mapMemory(staging.memory, 0, handle.size, {});
    std::memcpy(rbBuffer.data(), mapping, handle.size);
    device.unmapMemory(staging.memory);
    device.destroy(staging.buffer);
    device.free(staging.memory);
    return rbBuffer;
}

std::vector<uint8_t> Interface::readback(Texture texture)
{
    auto& handle = textures[texture];
    auto mr = device.getImageMemoryRequirements(handle.image);
    std::vector<uint8_t> rbBuffer{};
    rbBuffer.resize(mr.size);

    auto staging = allocateBuffer(mr.size, vk::BufferUsageFlagBits::eTransferDst,
                                  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    auto copyCmdBuffer = beginOneTimeCmdBuffer(graphicsCmdPool);
    transitionImageLayout(copyCmdBuffer, handle.image, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
    vk::BufferImageCopy region{0, 0, 0, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {}, handle.extent};
    copyCmdBuffer.copyImageToBuffer(handle.image, vk::ImageLayout::eTransferSrcOptimal, staging.buffer, {region});
    transitionImageLayout(copyCmdBuffer, handle.image, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);
    endOneTimeCmdBuffer(copyCmdBuffer, graphicsCmdPool, renderQueue);
    auto mapping = device.mapMemory(staging.memory, 0, mr.size, {});
    std::memcpy(rbBuffer.data(), mapping, mr.size);
    device.unmapMemory(staging.memory);
    device.destroy(staging.buffer);
    device.free(staging.memory);
    return rbBuffer;
}

uint32_t Interface::backbufferCount(Window window)
{
    return static_cast<uint32_t>(state->wsi.getWindow(window).imageViews.size());
}

uint32_t Interface::nextFrame(Window window) { return state->wsi.aquireNextImage(window, state->device); }

void Interface::pollEvents(Window window) { state->wsi.pollEvents(window); }

void Interface::present(Window window)
{
    auto& handle = wsi.getWindow(window);
    auto current = handle.currentFrameIndex;
    auto cmdBuffer = beginOneTimeCmdBuffer(graphicsCmdPool);
    transitionImageLayout(cmdBuffer, handle.images[current], vk::ImageLayout::eColorAttachmentOptimal,
                          vk::ImageLayout::ePresentSrcKHR);
    cmdBuffer.end();
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    renderQueue.submit(
        {{1, &handle.imageAvailableSemaphore, waitStages, 1, &cmdBuffer, 1, &handle.renderFinishedSemaphore}}, {});
    wsi.presentImage(window, renderQueue);
    renderQueue.waitIdle();
    device.freeCommandBuffers(graphicsCmdPool, 1, &cmdBuffer);
    cmdBuffer = beginOneTimeCmdBuffer(graphicsCmdPool);
    transitionImageLayout(cmdBuffer, handle.images[current], vk::ImageLayout::ePresentSrcKHR,
                          vk::ImageLayout::eColorAttachmentOptimal);
    endOneTimeCmdBuffer(cmdBuffer, graphicsCmdPool, renderQueue);
}

void Interface::setWindowTitle(Window window, const std::string& title) { wsi.setWindowTitle(window, title.c_str()); }

bool Interface::windowShouldClose(Window window) { return wsi.windowShouldClose(window); }

bool Interface::keyDown(Window window, Key key) { return wsi.keyDown(window, key); }

std::pair<int, int> Interface::mousePosition(Window window) { return wsi.mousePosition(window); }

std::pair<uint32_t, uint32_t> Interface::screenResolution() { return wsi.screenResolution(); }

void Interface::InternalState::free(Shader shader)
{
    device.waitIdle();
    auto& handle = shaders[shader];
    device.destroy(handle.module);
    shaders.erase(shader);
}
void Interface::InternalState::free(Buffer buffer)
{
    device.waitIdle();
    auto& handle = buffers[buffer];
    device.destroy(handle.buffer);
    device.free(handle.memory);
    buffers.erase(buffer);
}
void Interface::InternalState::free(Texture texture)
{
    device.waitIdle();
    auto& handle = textures[texture];
    auto& depthHandle = textureDepthBuffers[texture];
    if (depthHandle.image) {
        device.destroy(depthHandle.imageView);
        device.destroy(depthHandle.image);
        device.free(depthHandle.memory);
        textureDepthBuffers.erase(texture);
    }
    device.destroy(handle.sampler);
    device.destroy(handle.imageView);
    device.destroy(handle.image);
    device.free(handle.memory);
    textures.erase(texture);
}
void Interface::InternalState::free(Window window)
{
    device.waitIdle();
    auto& depthHandle = windowDepthBuffers[window];
    if (depthHandle.image) {
        device.destroy(depthHandle.imageView);
        device.destroy(depthHandle.image);
        device.free(depthHandle.memory);
        windowDepthBuffers.erase(window);
    }
    wsi.free(window, instance, device);
}
void Interface::InternalState::free(InputSet inputSet)
{
    device.waitIdle();
    auto& handle = inputSets[inputSet];
    device.destroy(handle.descriptorPool);
    inputSets.erase(inputSet);
}
void Interface::InternalState::free(RenderPass renderPass)
{
    device.waitIdle();
    auto& handle = renderPasses[renderPass];
    for (auto& fb : handle.framebuffers) device.destroy(fb);
    device.destroy(handle.renderPass);
    for (auto& sl : handle.setLayouts) device.destroy(sl);
    device.destroy(handle.pipeline);
    device.destroy(handle.pipelineLayout);
    renderPasses.erase(renderPass);
}
void Interface::InternalState::free(CommandBuffer commandBuffer)
{
    device.waitIdle();
    auto& handle = commandBuffers[commandBuffer];
    device.freeCommandBuffers(graphicsCmdPool, {handle.cmdBuffer});
    commandBuffers.erase(commandBuffer);
}

Buffer_vkData Interface::InternalState::allocateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                       vk::MemoryPropertyFlags properties)
{
    // TODO use VMA
    vk::Buffer buffer = device.createBuffer({{}, size, usage, vk::SharingMode::eExclusive, 1, &renderQueueFamiliy});
    auto mr = device.getBufferMemoryRequirements(buffer);
    vk::DeviceMemory memory = device.allocateMemory({mr.size, findMemoryType(mr.memoryTypeBits, properties)});
    device.bindBufferMemory(buffer, memory, 0);
    return {buffer, memory, usage, size};
}

vk::Format Interface::InternalState::findDepthFormat()
{
    std::array<vk::Format, 3> candidates{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
                                         vk::Format::eD24UnormS8Uint};
    vk::FormatFeatureFlags features = vk::FormatFeatureFlagBits::eDepthStencilAttachment;
    for (auto format : candidates) {
        auto props = pDevice.getFormatProperties(format);
        if ((props.optimalTilingFeatures & features) == features) return format;
    }
    throw std::runtime_error("[TGA Vulkan] Required Depth Format not present on this system");
}

DepthBuffer_vkData Interface::InternalState::createDepthBuffer(uint32_t width, uint32_t height)
{
    static vk::Format depthFormat = findDepthFormat();
    vk::Image image = device.createImage({{},
                                          vk::ImageType::e2D,
                                          depthFormat,
                                          {width, height, 1},
                                          1,
                                          1,
                                          vk::SampleCountFlagBits::e1,
                                          vk::ImageTiling::eOptimal,
                                          vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                          vk::SharingMode::eExclusive});
    auto mr = device.getImageMemoryRequirements(image);
    vk::DeviceMemory memory =
        device.allocateMemory({mr.size, findMemoryType(mr.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)});
    device.bindImageMemory(image, memory, 0);
    vk::ImageView view = device.createImageView(
        {{}, image, vk::ImageViewType::e2D, depthFormat, {}, {vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1}});
    auto transitionCmdBuffer = beginOneTimeCmdBuffer(graphicsCmdPool);
    transitionImageLayout(transitionCmdBuffer, image, vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eDepthStencilAttachmentOptimal);
    endOneTimeCmdBuffer(transitionCmdBuffer, graphicsCmdPool, renderQueue);
    return {image, view, memory};
}

vk::RenderPass Interface::InternalState::makeRenderPass(std::vector<vk::Format> const& colorFormats,
                                                        ClearOperation clearOps, vk::ImageLayout layout)
{
    auto colorLoadOp = vk::AttachmentLoadOp::eLoad;
    auto depthLoadOp = vk::AttachmentLoadOp::eLoad;
    if (clearOps == ClearOperation::all || clearOps == ClearOperation::color)
        colorLoadOp = vk::AttachmentLoadOp::eClear;
    if (clearOps == ClearOperation::all || clearOps == ClearOperation::depth)
        depthLoadOp = vk::AttachmentLoadOp::eClear;
    std::vector<vk::AttachmentDescription> attachments;
    attachments.reserve(colorFormats.size());
    for (auto colorFormat : colorFormats) {
        vk::AttachmentDescription()
            .setFormat(colorFormat)
            .setLoadOp(colorLoadOp)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setInitialLayout(layout)
            .setFinalLayout(layout);
        attachments.push_back({{},
                               colorFormat,
                               vk::SampleCountFlagBits::e1,
                               colorLoadOp,
                               vk::AttachmentStoreOp::eStore,
                               vk::AttachmentLoadOp::eDontCare,
                               vk::AttachmentStoreOp::eDontCare,
                               layout,
                               layout});
    }
    attachments.push_back({{},
                           findDepthFormat(),
                           vk::SampleCountFlagBits::e1,
                           depthLoadOp,
                           vk::AttachmentStoreOp::eStore,
                           vk::AttachmentLoadOp::eDontCare,
                           vk::AttachmentStoreOp::eDontCare,
                           vk::ImageLayout::eDepthStencilAttachmentOptimal,
                           vk::ImageLayout::eDepthStencilAttachmentOptimal});

    std::vector<vk::AttachmentReference> colorAttachmentRefs;
    colorAttachmentRefs.reserve(colorFormats.size());
    for (size_t i = 0; i < colorFormats.size(); ++i)
        colorAttachmentRefs.push_back({static_cast<uint32_t>(i), vk::ImageLayout::eColorAttachmentOptimal});
    vk::AttachmentReference depthAttachmentRef{static_cast<uint32_t>(colorAttachmentRefs.size()),
                                               vk::ImageLayout::eDepthStencilAttachmentOptimal};
    vk::SubpassDescription subpass{{},
                                   vk::PipelineBindPoint::eGraphics,
                                   0,
                                   0,
                                   static_cast<uint32_t>(colorAttachmentRefs.size()),
                                   colorAttachmentRefs.data(),
                                   nullptr,
                                   &depthAttachmentRef};

    vk::PipelineStageFlags pipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubpassDependency subDependency{
        VK_SUBPASS_EXTERNAL, 0,  pipelineStageFlags,
        pipelineStageFlags,  {}, vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite};
    return device.createRenderPass(
        {{}, uint32_t(attachments.size()), attachments.data(), 1, &subpass, 1, &subDependency});
}

std::vector<vk::DescriptorSetLayout> Interface::InternalState::decodeInputLayout(const InputLayout& inputLayout)
{
    std::vector<vk::DescriptorSetLayout> descSetLayouts{};
    for (const auto& setLayout : inputLayout.setLayouts) {
        std::vector<vk::DescriptorSetLayoutBinding> bindings{};
        for (uint32_t i = 0; i < setLayout.bindingLayouts.size(); i++) {
            bindings.emplace_back(
                vk::DescriptorSetLayoutBinding{i, determineDescriptorType(setLayout.bindingLayouts[i].type),
                                               setLayout.bindingLayouts[i].count, vk::ShaderStageFlagBits::eAll});
        }
        descSetLayouts.emplace_back(device.createDescriptorSetLayout({{}, uint32_t(bindings.size()), bindings.data()}));
    }
    return descSetLayouts;
}

vk::Pipeline Interface::InternalState::makeGraphicsPipeline(const RenderPassInfo& renderPassInfo,
                                                            vk::PipelineLayout pipelineLayout,
                                                            vk::RenderPass renderPass)
{
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
    for (auto& stage : renderPassInfo.shaderStages) {
        auto& shader = shaders[stage];
        shaderStages.emplace_back(
            vk::PipelineShaderStageCreateInfo({}, determineShaderStage(shader.type), shader.module, "main"));
    }
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
    vk::Bool32 depthWrite = (!renderPassInfo.perPixelOperations.blendEnabled) && depthTest;

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
    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments(numBlendAttachemnts, colorBlendAttachment);

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
}
std::pair<vk::Pipeline, vk::PipelineBindPoint> Interface::InternalState::makePipeline(
    const RenderPassInfo& renderPassInfo, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass)
{
    bool isValid = renderPassInfo.shaderStages.size() > 0;
    bool vertexPresent{false};
    bool fragmentPresent{false};
    for (auto stage : renderPassInfo.shaderStages) {
        const auto& shader = shaders[stage];
        if (shader.type == ShaderType::compute) {
            if (renderPassInfo.shaderStages.size() == 1) {
                return {
                    device
                        .createComputePipeline(
                            {}, {{}, {{}, vk::ShaderStageFlagBits::eCompute, shader.module, "main"}, pipelineLayout})
                        .value,
                    vk::PipelineBindPoint::eCompute};
            } else {
                isValid = false;
                break;
            }
        } else if (shader.type == ShaderType::vertex) {
            isValid = isValid && !fragmentPresent && !vertexPresent;
            vertexPresent = true;
        } else if (shader.type == ShaderType::fragment) {
            isValid = isValid && !fragmentPresent && vertexPresent;
            fragmentPresent = true;
        } else {
            isValid = false;
            break;
        }
    }
    if (!isValid) throw std::runtime_error("[TGA Vulkan] Invalid Shader Stage Configuration");
    return {makeGraphicsPipeline(renderPassInfo, pipelineLayout, renderPass), vk::PipelineBindPoint::eGraphics};
}

vk::CommandBuffer Interface::InternalState::beginOneTimeCmdBuffer(vk::CommandPool& cmdPool)
{
    vk::CommandBuffer cmdBuffer = device.allocateCommandBuffers({cmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
    cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    return cmdBuffer;
}
void Interface::InternalState::endOneTimeCmdBuffer(vk::CommandBuffer& cmdBuffer, vk::CommandPool& cmdPool,
                                                   vk::Queue& submitQueue)
{
    cmdBuffer.end();
    submitQueue.submit({{0, nullptr, nullptr, 1, &cmdBuffer}}, {});
    submitQueue.waitIdle();
    device.freeCommandBuffers(cmdPool, 1, &cmdBuffer);
}

void Interface::InternalState::fillBuffer(size_t size, const uint8_t* data, uint32_t offset, vk::Buffer target)
{
    auto copyCmdBuffer = beginOneTimeCmdBuffer(transferCmdPool);
    if (size <= 65536 && (size % 4) == 0)  // Quick Path
    {
        copyCmdBuffer.updateBuffer(target, offset, size, data);
        endOneTimeCmdBuffer(copyCmdBuffer, transferCmdPool, renderQueue);
    } else  // Staging Buffer
    {
        auto buffer =
            allocateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                           vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        auto mapping = device.mapMemory(buffer.memory, 0, size, {});
        std::memcpy(mapping, data, size);
        device.unmapMemory(buffer.memory);
        vk::BufferCopy region{0, offset, size};
        copyCmdBuffer.copyBuffer(buffer.buffer, target, {region});
        endOneTimeCmdBuffer(copyCmdBuffer, transferCmdPool, renderQueue);
        device.destroy(buffer.buffer);
        device.free(buffer.memory);
    }
}

void Interface::InternalState::fillTexture(size_t size, const uint8_t* data, vk::Extent3D extent, uint32_t layers,
                                           vk::Image target)
{
    auto buffer = allocateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc,
                                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    auto mapping = device.mapMemory(buffer.memory, 0, size, {});
    std::memcpy(mapping, data, size);
    device.unmapMemory(buffer.memory);

    auto uploadCmd = device.allocateCommandBuffers({graphicsCmdPool, vk::CommandBufferLevel::ePrimary, 1})[0];
    uploadCmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    uploadCmd.copyBufferToImage(buffer.buffer, target, vk::ImageLayout::eTransferDstOptimal,
                                vk::BufferImageCopy()
                                    .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layers})
                                    .setImageExtent(extent));

    uploadCmd.end();
    auto cmdCompleteSignal = device.createSemaphore({});
    renderQueue.submit(vk::SubmitInfo().setCommandBuffers(uploadCmd).setSignalSemaphores(cmdCompleteSignal));
    device.waitSemaphores(vk::SemaphoreWaitInfo().setSemaphores(cmdCompleteSignal),
                          std::numeric_limits<uint64_t>::max());
    device.freeCommandBuffers(graphicsCmdPool, 1, &uploadCmd);
    device.destroySemaphore(cmdCompleteSignal);

    device.destroy(buffer.buffer);
    device.free(buffer.memory);
}

}  // namespace tga