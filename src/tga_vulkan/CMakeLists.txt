
find_package(Vulkan REQUIRED)

add_subdirectory(WSI_glfw)
set(TGA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")
#set(TGA_LIBRARY_HEADERS ${TGA_INCLUDE_DIR}/tga/tga.hpp ${TGA_INCLUDE_DIR}/tga/tga_vulkan/tga_vulkan.hpp)

add_library(tga_vulkan tga_vulkan.cpp tga_vulkan_extensions.cpp)#${TGA_LIBRARY_HEADERS})
target_include_directories(tga_vulkan PRIVATE Vulkan::Vulkan)
target_link_libraries(tga_vulkan PUBLIC Vulkan::Vulkan)
target_link_libraries(tga_vulkan PRIVATE tga_vulkan_wsi)
target_include_directories(tga_vulkan PUBLIC ${TGA_INCLUDE_DIR})
target_include_directories(tga_vulkan PUBLIC ${GLM_INCLUDE_DIRS})

#include(CheckIPOSupported)
#check_ipo_supported(RESULT ipo_result OUTPUT ipo_output)
#if(ipo_result)
#    set_property(TARGET tga_vulkan PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
#else()
#    message(WARNING "IPO not enabled: ${ipo_output}")
#endif()