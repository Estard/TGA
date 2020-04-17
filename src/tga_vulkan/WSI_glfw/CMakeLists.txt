
find_package(Vulkan REQUIRED)

include_directories(${GLFW_INCLUDE_DIRS})
add_library(tga_vulkan_wsi tga_vulkan_wsi.cpp)
target_include_directories(tga_vulkan_wsi PRIVATE Vulkan::Vulkan)
target_link_libraries(tga_vulkan_wsi PUBLIC Vulkan::Vulkan ${GLFW_LIBRARIES})
target_include_directories(tga_vulkan_wsi PUBLIC ../../../include)