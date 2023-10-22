#pragma once
#include "tga/tga_hash.hpp"
#include "tga/tga_vulkan/tga_vulkan_metadata.hpp"

namespace tga
{
    struct VulkanWSI {

        VulkanWSI();
        ~VulkanWSI();

        std::vector<const char*> getRequiredExtensions() const;
        Window createWindow(const WindowInfo&, vk::Instance&, vk::PhysicalDevice&, vk::Device&, uint32_t queueFamily);
        void setWindowTitle(Window window, const char* title);
        void free(Window window, vk::Instance&, vk::Device&);
        void pollEvents(Window window);

        bool windowShouldClose(Window window);

        bool keyDown(Window window, Key key);
        std::pair<int, int> mousePosition(Window window);

        std::pair<uint32_t, uint32_t> screenResolution() const;

        vkData::Window& getWindow(Window window);

        // window handle to data
        std::unordered_map<Window, vkData::Window> windows;
    };

};  // namespace tga
