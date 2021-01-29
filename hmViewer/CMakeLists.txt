

add_executable(heightmapDemo musterloesung.cpp HeightmapViewer.hpp HeightmapViewer.cpp CameraController.cpp hm.cpp)
target_link_libraries(heightmapDemo PUBLIC tga_vulkan ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(heightmapDemo PUBLIC ${PROJECT_SOURCE_DIR}/external)
if(WIN32)
    set_property(TARGET sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif(WIN32)
