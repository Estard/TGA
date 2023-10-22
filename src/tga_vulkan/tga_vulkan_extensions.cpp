#include "tga/tga_vulkan/tga_vulkan_extensions.hpp"

/*
Macros to reduce repeated typing of long function names
*/
#define PFN_FUN(RET, FUNC, ARGS)  \
    static PFN_##FUNC pfn_##FUNC; \
                                  \
    VKAPI_ATTR RET VKAPI_CALL FUNC ARGS

#define PFN_INIT(FROM, FUNC) pfn_##FUNC = reinterpret_cast<PFN_##FUNC>(FROM.getProcAddr(#FUNC))

#define PFN_INIT_REQUIRED(FROM, FUNC) \
    PFN_INIT(FROM, FUNC);             \
    if (!pfn_##FUNC) throw std::runtime_error(#FROM ".getProcAddr: Unable to find function " #FUNC)

PFN_FUN(VkResult, vkCreateDebugUtilsMessengerEXT,
        (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
         const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pMessenger))
{
    return pfn_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

PFN_FUN(void, vkDestroyDebugUtilsMessengerEXT,
        (VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const *pAllocator))
{
    return pfn_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

namespace tga
{
    void loadVkInstanceExtensions(vk::Instance &instance)
    {
        PFN_INIT_REQUIRED(instance, vkCreateDebugUtilsMessengerEXT);
        PFN_INIT_REQUIRED(instance, vkDestroyDebugUtilsMessengerEXT);
    }
}  // namespace tga

// Device extensions. Mostly for ray tracing

PFN_FUN(VkResult, vkCreateAccelerationStructureKHR,
        (VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
         const VkAllocationCallbacks *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure))
{
    return pfn_vkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

PFN_FUN(void, vkDestroyAccelerationStructureKHR,
        (VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks *pAllocator))
{
    return pfn_vkDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

PFN_FUN(void, vkGetAccelerationStructureBuildSizesKHR,
        (VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
         const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo, const uint32_t *pMaxPrimitiveCounts,
         VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo))
{
    return pfn_vkGetAccelerationStructureBuildSizesKHR(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

PFN_FUN(VkDeviceAddress, vkGetAccelerationStructureDeviceAddressKHR,
        (VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR *pInfo))
{
    return pfn_vkGetAccelerationStructureDeviceAddressKHR(device, pInfo);
}

PFN_FUN(void, vkCmdBuildAccelerationStructuresKHR,
        (VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos,
         const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos))
{
    return pfn_vkCmdBuildAccelerationStructuresKHR(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}

namespace tga
{
    void loadVkDeviceExtensions(vk::Device &device)
    {
        PFN_INIT(device, vkCreateAccelerationStructureKHR);
        PFN_INIT(device, vkDestroyAccelerationStructureKHR);
        PFN_INIT(device, vkGetAccelerationStructureDeviceAddressKHR);
        PFN_INIT(device, vkGetAccelerationStructureBuildSizesKHR);
        PFN_INIT(device, vkCmdBuildAccelerationStructuresKHR);

        if(
            !pfn_vkCreateAccelerationStructureKHR ||
            !pfn_vkDestroyAccelerationStructureKHR ||
            !pfn_vkGetAccelerationStructureDeviceAddressKHR ||
            !pfn_vkGetAccelerationStructureBuildSizesKHR ||
            !pfn_vkCmdBuildAccelerationStructuresKHR
        )
        std::cerr << "Ray Tracing not available\n";
    }

}  // namespace tga