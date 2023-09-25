#pragma once
#include <iostream>

#include "vulkan/vulkan.hpp"


namespace tga{
    void loadVkInstanceExtensions(vk::Instance&);
    void loadVkDeviceExtensions(vk::Device &);
}