#include "tga/tga_vulkan/tga_vulkan_WSI.hpp"
#include "GLFW/glfw3.h"
#include <iostream>

namespace tga
{
    VulkanWSI::VulkanWSI()
    {
        glfwInit();
    }
    VulkanWSI::~VulkanWSI()
    {
        glfwTerminate();
    }
    void VulkanWSI::setVulkanHandles(vk::Instance _instance, vk::PhysicalDevice _pDevice, vk::Device _device, vk::Queue _presentQueue, uint32_t _queueFamiliy)
    {
        instance = _instance;
        pDevice = _pDevice;
        device = _device;
        presentQueue = _presentQueue;
        queueFamiliy = _queueFamiliy;
    }

    std::vector<const char*> VulkanWSI::getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions;
        for (uint32_t i = 0; i < glfwExtensionCount; i++)
            extensions.push_back(glfwExtensions[i]);
        return extensions;
    }

    Window VulkanWSI::createWindow(const WindowInfo &windowInfo)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        GLFWwindow* glfwWindow = glfwCreateWindow( int(windowInfo.width),int(windowInfo.height), "TGA VULKAN", nullptr, nullptr);

        VkSurfaceKHR vkSurface;
        if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, &vkSurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        vk::SurfaceKHR surface{vkSurface};
        if(!pDevice.getSurfaceSupportKHR(queueFamiliy,surface)){
            instance.destroy(surface);
            throw std::runtime_error("Cannot use graphics Queue to present to Window");
        }

        auto surfaceFormat = chooseSurfaceFormat(surface);
        auto presentMode = choosePresentMode(surface, windowInfo.presentMode);
        auto extent = chooseSwapExtent(surface, windowInfo);
        auto capabilities = pDevice.getSurfaceCapabilitiesKHR(surface);
        uint32_t imageCount = std::max(windowInfo.framebufferCount,capabilities.minImageCount);
        if(capabilities.maxImageCount)
            imageCount = std::min(imageCount,capabilities.maxImageCount);

        auto swapchain = device.createSwapchainKHR({{},surface,imageCount,surfaceFormat.format,surfaceFormat.colorSpace,extent,1,
            vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive,1,&queueFamiliy,capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,presentMode,VK_TRUE});

        std::vector<vk::Image> images = device.getSwapchainImagesKHR(swapchain);
        std::vector<vk::ImageView> imageViews{};
        std::vector<vk::Fence> fences{};
        std::vector<vk::Semaphore> availabilitySemas{};
        std::vector<vk::Semaphore> renderSemas{};
        for(auto image : images){
            imageViews.emplace_back(device.createImageView({{},image,vk::ImageViewType::e2D,surfaceFormat.format,{},{vk::ImageAspectFlagBits::eColor,0,1,0,1}}));
         //   fences.emplace_back(device.createFence({{vk::FenceCreateFlagBits::eSignaled}}));
         //   availabilitySemas.emplace_back(device.createSemaphore({}));
         //   renderSemas.emplace_back(device.createSemaphore({}));
        }

        Window_TV window_tv{surface,swapchain,extent,surfaceFormat.format,images,imageViews,glfwWindow,
            device.createFence({vk::FenceCreateFlagBits::eSignaled}),device.createSemaphore({}),device.createSemaphore({}),0};
        Window window = Window(TgaWindow(glfwWindow));
        windows.emplace(window,window_tv);
        return window;
    }

    void VulkanWSI::free(Window window)
    {
        auto &handle = windows[window];
        for(auto &imageView : handle.imageViews)
            device.destroy(imageView);
        device.destroy(handle.swapchain);
        instance.destroy(handle.surface);
       // for(auto &fence : handle.inFlightFences)
            device.destroy(handle.inFlightFence);
       // for(auto &sema: handle.imageAvailableSemaphores)
            device.destroy(handle.imageAvailableSemaphore);
      //  for(auto &sema: handle.renderFinishedSemaphores)
            device.destroy(handle.renderFinishedSemaphore);

        glfwDestroyWindow(std::any_cast<GLFWwindow*>(handle.nativeHandle));
        windows.erase(window);
    }
    void VulkanWSI::setWindowTitle(Window window, const char* title)
    {
        auto& handle = getWindow(window);
        glfwSetWindowTitle(std::any_cast<GLFWwindow*>(handle.nativeHandle),title);
    }

    Window_TV& VulkanWSI::getWindow(Window window)
    {
        return windows[window];
    }

    uint32_t VulkanWSI::aquireNextImage(Window window) 
    {
        glfwPollEvents();
        auto &handle = windows[window];
        auto nextFrame = device.acquireNextImageKHR(handle.swapchain,std::numeric_limits<uint32_t>::max(),
        handle.imageAvailableSemaphore,vk::Fence());
        handle.currentFrameIndex = nextFrame.value;
        return handle.currentFrameIndex;
    }
    void VulkanWSI::presentImage(Window window)
    {
        auto &handle = windows[window];
        presentQueue.presentKHR({1,&handle.renderFinishedSemaphore,1,&handle.swapchain,&handle.currentFrameIndex});
    }

    bool VulkanWSI::windowShouldClose(Window window) 
    {
        auto &handle = windows[window];
        return glfwWindowShouldClose(std::any_cast<GLFWwindow*>(handle.nativeHandle));
    }

    int toGlfwKey(Key key);
    bool VulkanWSI::keyDown(Window window, Key key)
    {
        auto &handle = windows[window];
        GLFWwindow* glfwWin = std::any_cast<GLFWwindow*>(handle.nativeHandle);
        if(key == Key::MouseLeft)
            return glfwGetMouseButton(glfwWin,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if(key == Key::MouseMiddle)
            return glfwGetMouseButton(glfwWin,GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        if(key == Key::MouseRight)
            return glfwGetMouseButton(glfwWin,GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        auto glfwKey = toGlfwKey(key);
        return glfwGetKey(glfwWin,glfwKey) == GLFW_PRESS;
    }

    std::pair<int, int> VulkanWSI::mousePosition(Window window)
    {
        auto &handle = windows[window];
        GLFWwindow* glfwWin = std::any_cast<GLFWwindow*>(handle.nativeHandle);
        double xpos = 0;
        double ypos = 0;
        glfwGetCursorPos(glfwWin,&xpos,&ypos);
        return {int(xpos),int(ypos)};
    }


    vk::SurfaceFormatKHR VulkanWSI::chooseSurfaceFormat(vk::SurfaceKHR surface)
    {
        auto formats = pDevice.getSurfaceFormatsKHR(surface);
        for(const auto& format: formats){
            if(format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                return format;
        }
        return formats[0];
    }
    vk::PresentModeKHR VulkanWSI::choosePresentMode(vk::SurfaceKHR surface, PresentMode wantedPresentMode)
    {
        auto presentModes = pDevice.getSurfacePresentModesKHR(surface);
        auto presentMode = vk::PresentModeKHR::eFifo; //Always available
        if(wantedPresentMode == PresentMode::immediate){
            for(const auto &mode: presentModes){
                if(mode == vk::PresentModeKHR::eMailbox){
                    return mode; //Prefer Mailbox over Immediate
                }
                else if(mode == vk::PresentModeKHR::eImmediate){
                    presentMode = mode;
                }
            }
        }
        return presentMode;
    }
    vk::Extent2D VulkanWSI::chooseSwapExtent(vk::SurfaceKHR surface,const WindowInfo &windowInfo)
    {
        auto capabilities = pDevice.getSurfaceCapabilitiesKHR(surface);
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            return capabilities.currentExtent;
        return {std::clamp(windowInfo.width,capabilities.minImageExtent.width,capabilities.maxImageExtent.width),
                std::clamp(windowInfo.height,capabilities.minImageExtent.height,capabilities.maxImageExtent.height)};
    }


    int toGlfwKey(Key key)
    {
        switch (key) {
            case Key::Space: return GLFW_KEY_SPACE;
            case Key::Apostrophe: return GLFW_KEY_APOSTROPHE;
            case Key::Comma: return GLFW_KEY_COMMA;
            case Key::Minus: return GLFW_KEY_MINUS;
            case Key::Period: return GLFW_KEY_PERIOD;
            case Key::Slash: return GLFW_KEY_SLASH;
            case Key::n0: return GLFW_KEY_0;
            case Key::n1: return GLFW_KEY_1;
            case Key::n2: return GLFW_KEY_2;
            case Key::n3: return GLFW_KEY_3;
            case Key::n4: return GLFW_KEY_4;
            case Key::n5: return GLFW_KEY_5;
            case Key::n6: return GLFW_KEY_6;
            case Key::n7: return GLFW_KEY_7;
            case Key::n8: return GLFW_KEY_8;
            case Key::n9: return GLFW_KEY_9;
            case Key::Semicolon: return GLFW_KEY_SEMICOLON;
            case Key::Equal: return GLFW_KEY_EQUAL;
            case Key::A: return GLFW_KEY_A;
            case Key::B: return GLFW_KEY_B;
            case Key::C: return GLFW_KEY_C;
            case Key::D: return GLFW_KEY_D;
            case Key::E: return GLFW_KEY_E;
            case Key::F: return GLFW_KEY_F;
            case Key::G: return GLFW_KEY_G;
            case Key::H: return GLFW_KEY_H;
            case Key::I: return GLFW_KEY_I;
            case Key::J: return GLFW_KEY_J;
            case Key::K: return GLFW_KEY_K;
            case Key::L: return GLFW_KEY_L;
            case Key::M: return GLFW_KEY_M;
            case Key::N: return GLFW_KEY_N;
            case Key::O: return GLFW_KEY_O;
            case Key::P: return GLFW_KEY_P;
            case Key::Q: return GLFW_KEY_Q;
            case Key::R: return GLFW_KEY_R;
            case Key::S: return GLFW_KEY_S;
            case Key::T: return GLFW_KEY_T;
            case Key::U: return GLFW_KEY_U;
            case Key::V: return GLFW_KEY_V;
            case Key::W: return GLFW_KEY_W;
            case Key::X: return GLFW_KEY_X;
            case Key::Y: return GLFW_KEY_Y;
            case Key::Z: return GLFW_KEY_Z;
            case Key::Bracket_Left: return GLFW_KEY_LEFT_BRACKET;
            case Key::Backslash: return GLFW_KEY_BACKSLASH;
            case Key::Bracket_Right: return GLFW_KEY_RIGHT_BRACKET;
            case Key::Grave_Accent: return GLFW_KEY_GRAVE_ACCENT;
            case Key::World_1: return GLFW_KEY_WORLD_1;
            case Key::World_2: return GLFW_KEY_WORLD_2;        
            case Key::Escape: return GLFW_KEY_ESCAPE;
            case Key::Enter: return GLFW_KEY_ENTER;
            case Key::Tab: return GLFW_KEY_TAB;
            case Key::Backspace: return GLFW_KEY_BACKSPACE;
            case Key::Insert: return GLFW_KEY_INSERT;
            case Key::Delete: return GLFW_KEY_DELETE;
            case Key::Right: return GLFW_KEY_RIGHT;
            case Key::Left: return GLFW_KEY_LEFT;
            case Key::Down: return GLFW_KEY_DOWN;
            case Key::Up: return GLFW_KEY_UP;
            case Key::Page_Up: return GLFW_KEY_PAGE_UP;
            case Key::Page_Down: return GLFW_KEY_PAGE_DOWN;
            case Key::Home: return GLFW_KEY_HOME;
            case Key::End: return GLFW_KEY_END;
            case Key::Caps_Lock: return GLFW_KEY_CAPS_LOCK;
            case Key::Scroll_Lock: return GLFW_KEY_SCROLL_LOCK;
            case Key::Num_Lock: return GLFW_KEY_NUM_LOCK;
            case Key::Print_Screen: return GLFW_KEY_PRINT_SCREEN;
            case Key::Pause: return GLFW_KEY_PAUSE;
            case Key::F1: return GLFW_KEY_F1;
            case Key::F2: return GLFW_KEY_F2;
            case Key::F3: return GLFW_KEY_F3;
            case Key::F4: return GLFW_KEY_F4;
            case Key::F5: return GLFW_KEY_F5;
            case Key::F6: return GLFW_KEY_F6;
            case Key::F7: return GLFW_KEY_F7;
            case Key::F8: return GLFW_KEY_F8;
            case Key::F9: return GLFW_KEY_F9;
            case Key::F10: return GLFW_KEY_F10;
            case Key::F11: return GLFW_KEY_F11;
            case Key::F12: return GLFW_KEY_F12;
            case Key::F13: return GLFW_KEY_F13;
            case Key::F14: return GLFW_KEY_F14;
            case Key::F15: return GLFW_KEY_F15;
            case Key::F16: return GLFW_KEY_F16;
            case Key::F17: return GLFW_KEY_F17;
            case Key::F18: return GLFW_KEY_F18;
            case Key::F19: return GLFW_KEY_F19;
            case Key::F20: return GLFW_KEY_F20;
            case Key::F21: return GLFW_KEY_F21;
            case Key::F22: return GLFW_KEY_F22;
            case Key::F23: return GLFW_KEY_F23;
            case Key::F24: return GLFW_KEY_F24;
            case Key::F25: return GLFW_KEY_F25;
            case Key::KP_0: return GLFW_KEY_KP_0;
            case Key::KP_1: return GLFW_KEY_KP_1;
            case Key::KP_2: return GLFW_KEY_KP_2;
            case Key::KP_3: return GLFW_KEY_KP_3;
            case Key::KP_4: return GLFW_KEY_KP_4;
            case Key::KP_5: return GLFW_KEY_KP_5;
            case Key::KP_6: return GLFW_KEY_KP_6;
            case Key::KP_7: return GLFW_KEY_KP_7;
            case Key::KP_8: return GLFW_KEY_KP_8;
            case Key::KP_9: return GLFW_KEY_KP_9;
            case Key::KP_Decimal: return GLFW_KEY_KP_DECIMAL;
            case Key::KP_Divide: return GLFW_KEY_KP_DIVIDE;
            case Key::KP_Multiply: return GLFW_KEY_KP_MULTIPLY;
            case Key::KP_Subtract: return GLFW_KEY_KP_SUBTRACT;
            case Key::KP_Add: return GLFW_KEY_KP_ADD;
            case Key::KP_Enter: return GLFW_KEY_KP_ENTER;
            case Key::KP_Equal: return GLFW_KEY_KP_EQUAL;
            case Key::Shift_Left: return GLFW_KEY_LEFT_SHIFT;
            case Key::Control_Left: return GLFW_KEY_LEFT_CONTROL;
            case Key::Alt_Left: return GLFW_KEY_LEFT_ALT;
            case Key::Super_Left: return GLFW_KEY_LEFT_SUPER;
            case Key::Shift_Right: return GLFW_KEY_RIGHT_SHIFT;
            case Key::Control_Right: return GLFW_KEY_RIGHT_CONTROL;
            case Key::Alt_Right: return GLFW_KEY_RIGHT_ALT;
            case Key::Super_Right: return GLFW_KEY_RIGHT_SUPER;
            case Key::Menu: return GLFW_KEY_MENU;
            default: return GLFW_KEY_UNKNOWN;
        }
    }
}