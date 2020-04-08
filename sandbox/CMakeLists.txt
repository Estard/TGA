add_executable(sandbox sandbox.cpp)
target_link_libraries(sandbox PUBLIC tga_vulkan)
target_include_directories(tga_vulkan PUBLIC ../include)
