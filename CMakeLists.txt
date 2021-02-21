cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(tga VERSION 1.0 LANGUAGES C CXX DESCRIPTION "Trainings Graphics API")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Og")
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
find_package(GLFW)
if(NOT GLFW_FOUND)
	if(WIN32)
		message(STATUS "GLFW could not be found with normal lookup, building libraries")
		set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
		add_subdirectory(external/glfw)
		add_subdirectory(external/glm)
		set(GLFW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)
		set(GLFW_LIBRARIES glfw)
	else()
		message(STATUS "GLFW could not be found with normal lookup, use PkgConfig instead")
		find_package(PkgConfig REQUIRED)
		pkg_search_module(GLFW REQUIRED glfw3)
	endif()
else()
	set(GLFW_LIBRARIES ${GLFW_LIBRARY})
	set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
endif()
# copy shared libraries
# not necessary when building libraries
if (WIN32)
	# glfw dll
	# get_filename_component(GLFW_BINARY_DIRECTORY ${GLFW_LIBRARY} DIRECTORY)
	# file(COPY ${GLFW_BINARY_DIRECTORY}/glfw3.dll DESTINATION ${CMAKE_BINARY_DIR}/)
endif()

add_subdirectory(src)
add_subdirectory(sandbox)
add_subdirectory(hmViewer)

