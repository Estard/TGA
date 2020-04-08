
find_package(Vulkan REQUIRED)
add_subdirectory(WSI_glfw)
add_library(tga_vulkan tga_vulkan.cpp)
target_include_directories(tga_vulkan PRIVATE Vulkan::Vulkan)
target_link_libraries(tga_vulkan PUBLIC Vulkan::Vulkan)
target_link_libraries(tga_vulkan PRIVATE tga_vulkan_wsi)
target_include_directories(tga_vulkan PUBLIC ../../include)