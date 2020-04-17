cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(tga VERSION 1.0 LANGUAGES C CXX DESCRIPTION "Trainings Graphics API")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif(CMAKE_COMPILER_IS_GNUCXX)
if(WIN32)
    add_definitions(-DNOMINMAX)
endif(WIN32)

find_package(Vulkan REQUIRED)

cmake_policy(SET CMP0074 NEW) # use _ROOT environment variables
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(GLM)
find_package(GLFW)
if(NOT GLFW_FOUND)
	message(STATUS "GLFW could not be found with normal lookup, use PkgConfig instead")
	find_package(PkgConfig REQUIRED)
	pkg_search_module(GLFW REQUIRED glfw3)
else()
	set(GLFW_LIBRARIES ${GLFW_LIBRARY})
	set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
endif()
# copy shared libraries
if (WIN32)
	# glfw dll
	get_filename_component(GLFW_BINARY_DIRECTORY ${GLFW_LIBRARY} DIRECTORY)
	file(COPY ${GLFW_BINARY_DIRECTORY}/glfw3.dll DESTINATION ${CMAKE_BINARY_DIR}/)
endif()

add_subdirectory(src)
add_subdirectory(sandbox)


