find_package(Threads)
add_executable(sandbox sandbox.cpp)
target_link_libraries(sandbox PUBLIC tga_vulkan ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(tga_vulkan PUBLIC ../include)
if(WIN32)
    # working directory is the root, not build/ or whatever was specified when launching CMake
    # this allows to find the shaders via relative paths
    set_property(TARGET sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif(WIN32)


add_executable(terrainViewer terrainViewer.cpp)
target_link_libraries(terrainViewer PUBLIC tga_vulkan ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
    # working directory is the root, not build/ or whatever was specified when launching CMake
    # this allows to find the shaders via relative paths
    set_property(TARGET terrainViewer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif(WIN32)

add_executable(particles Particles.cpp)
target_link_libraries(particles PUBLIC tga_vulkan ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
    # working directory is the root, not build/ or whatever was specified when launching CMake
    # this allows to find the shaders via relative paths
    set_property(TARGET particles PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif(WIN32)


