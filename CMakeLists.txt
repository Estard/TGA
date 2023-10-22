cmake_minimum_required(VERSION 3.9.4 FATAL_ERROR)
project(tga VERSION 2.0 LANGUAGES C CXX DESCRIPTION "Trainings Graphics API")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Og")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
endif(CMAKE_COMPILER_IS_GNUCXX)
if(WIN32)
    add_definitions(-DNOMINMAX)
endif(WIN32)

find_package(Vulkan REQUIRED)

cmake_policy(SET CMP0074 NEW) # use _ROOT environment variables
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# When not explicitly set use submodules
if(NOT(DEFINED GLM_ROOT_DIR))
set(GLM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)
endif()
if(NOT(DEFINED GLFW_ROOT_DIR))
set(GLFW_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
endif()

find_package(GLM)

if(NOT GLM_FOUND)
	message(STATUS "GLM not found, including submodule explizitly")
	add_subdirectory(external/glm)
	set(GLM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)
endif()

find_package(GLFW)
if(NOT GLFW_FOUND)

	find_package(PkgConfig)
	if(PkgConfig_FOUND)
		message(STATUS "GLFW could not be found with normal lookup, try PkgConfig instead")
		pkg_search_module(GLFW glfw3)
	else()
		message(STATUS "PkgConfig not found")
	endif()

	if(NOT GLFW_FOUND)
		message(STATUS "GLFW could not be found, building libraries")
		set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
		add_subdirectory(external/glfw)
		set(GLFW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)
		set(GLFW_LIBRARIES glfw)
	endif()

else()
	set(GLFW_LIBRARIES ${GLFW_LIBRARY})
	set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
endif()


add_subdirectory(src)

option(TGA_BUILD_EXAMPLES on)

if(TGA_BUILD_EXAMPLES)
	message("Set TGA_BUILD_EXAMPLES to ${TGA_BUILD_EXAMPLES}: including subdirectory")
	add_subdirectory(examples)
endif()