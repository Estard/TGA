
find_package(Vulkan REQUIRED)
find_package(GLM REQUIRED)

add_subdirectory(WSI_glfw)
set(TGA_INCLUDE_DIR ../../include)
set(TGA_LIBRARY_HEADERS ${TGA_INCLUDE_DIR}/tga/tga.hpp ${TGA_INCLUDE_DIR}/tga/tga_vulkan/tga_vulkan.hpp)

add_library(tga_vulkan tga_vulkan.cpp ${TGA_LIBRARY_HEADERS})
target_include_directories(tga_vulkan PRIVATE Vulkan::Vulkan)
target_link_libraries(tga_vulkan PUBLIC Vulkan::Vulkan)
target_link_libraries(tga_vulkan PRIVATE tga_vulkan_wsi)
target_include_directories(tga_vulkan PUBLIC ${TGA_INCLUDE_DIR}})
target_include_directories(tga_vulkan PUBLIC ${GLM_INCLUDE_DIRS})
