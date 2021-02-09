add_library(tga_utils tga_utils.cpp)
target_include_directories(tga_utils PUBLIC ${PROJECT_SOURCE_DIR}/external)
target_link_libraries(tga_utils PUBLIC tga_vulkan)