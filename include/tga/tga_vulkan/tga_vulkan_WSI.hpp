#pragma once
#include "tga/tga_WSI.hpp"
#include "tga/tga_hash.hpp"
#include "vulkan/vulkan.hpp"

namespace tga{


    struct Window_TV{
        vk::SurfaceKHR surface;
        vk::SwapchainKHR swapchain;
        vk::Extent2D extent;
        vk::Format format;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        std::any nativeHandle;
        vk::Fence inFlightFence;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
        uint32_t currentFrameIndex;
    };


    class VulkanWSI : public WSI
    {
        public:
        VulkanWSI();
        ~VulkanWSI();
        Window createWindow(const WindowInfo &windowInfo) override;
        void setWindowTitle(Window window, const char* title) override;
        void free(Window window) override;
        uint32_t aquireNextImage(Window window) override;
        void pollEvents(Window window) override;
        void presentImage(Window window) override;

        bool windowShouldClose(Window window) override;

        void setVulkanHandles(vk::Instance _instance, vk::PhysicalDevice _pDevice, vk::Device _device, vk::Queue _presentQueue, uint32_t _queueFamiliy);
        std::vector<const char*> getRequiredExtensions();

        bool keyDown(Window window, Key key) override;
        std::pair<int, int> mousePosition(Window window) override;

        std::pair<uint32_t, uint32_t> screenResolution() override;



        //Extras
        Window_TV& getWindow(Window window);
        std::unordered_map<Window,Window_TV> windows;
        private:
        vk::Instance instance;
        vk::PhysicalDevice pDevice;
        vk::Device device;
        vk::Queue presentQueue;
        uint32_t queueFamiliy;

        vk::SurfaceFormatKHR chooseSurfaceFormat(vk::SurfaceKHR surface);
        vk::PresentModeKHR choosePresentMode(vk::SurfaceKHR surface, PresentMode wantedPresentMode);
        vk::Extent2D chooseSwapExtent(vk::SurfaceKHR surface,const WindowInfo &windowInfo);
        

    };

};
