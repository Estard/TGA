

add_executable(heightmapDemo musterloesung.cpp HeightmapViewer.hpp HeightmapViewer.cpp CameraController.cpp)
target_link_libraries(heightmapDemo PUBLIC tga_vulkan ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(heightmapDemo PUBLIC ../external)
if(WIN32)
    set_property(TARGET sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif(WIN32)
