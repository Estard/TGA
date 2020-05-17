#pragma once
#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

#include <chrono>
#include <sstream>
#include <thread>
#include <random>
#include <future>
#include <list>

#include "tga/tga_math.h"


//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  https://github.com/ashima/webgl-noise
//  Port from GLSL to C++ with GLM
static glm::vec3 mod289(const glm::vec3 &x) { return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f; }
static glm::vec2 mod289(const glm::vec2 &x) { return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f; }
static glm::vec3 permute(const glm::vec3 &x) { return mod289(((x*34.0f)+1.0f)*x); }
static float snoise(const glm::vec2 &v) {

	// Precompute values for skewed triangular grid
	const glm::vec4 C(0.211324865405187,
	                    // (3.0-sqrt(3.0))/6.0
	                    0.366025403784439,
	                    // 0.5*(sqrt(3.0)-1.0)
	                    -0.577350269189626,
	                    // -1.0 + 2.0 * C.x
	                    0.024390243902439);
	                    // 1.0 / 41.0
	// First corner (x0)
    glm::vec2 Cxx(C.x,C.x);
    glm::vec2 Czz(C.z,C.z);
    glm::vec3 Cwww(C.w);
	glm::vec2 i  = floor(v + dot(v, glm::vec2(C.y,C.y)));
	glm::vec2 x0 = v - i + dot(i, Cxx);
	// Other two corners (x1, x2)
	glm::vec2 i1 = glm::vec2(0.0);
	i1 = (x0.x > x0.y)? glm::vec2(1.0, 0.0):glm::vec2(0.0, 1.0);
	glm::vec2 x1 = glm::vec2(x0.x,x0.y) + Cxx - i1;
	glm::vec2 x2 = x0 + Czz;
	// Do some permutations to avoid
	// truncation effects in permutation
	i = mod289(i);
	glm::vec3 p = permute(
	        permute( i.y + glm::vec3(0.0, i1.y, 1.0))
	            + i.x + glm::vec3(0.0, i1.x, 1.0 ));
	glm::vec3 m = glm::max(glm::vec3(.5f) - glm::vec3(
	                    dot(x0,x0),
	                    dot(x1,x1),
	                    dot(x2,x2)
	                    ), 0.f);
	m = m*m ;
	m = m*m ;
	// Gradients:
	//  41 pts uniformly over a line, mapped onto a diamond
	//  The ring size 17*17 = 289 is close to a multiple
	//      of 41 (41*7 = 287)
	glm::vec3 x = 2.0f * glm::fract(p * Cwww) - 1.0f;
	glm::vec3 h = abs(x) - glm::vec3(0.5);
	glm::vec3 ox = floor(x + 0.5f);
	glm::vec3 a0 = x - ox;
	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt(a0*a0 + h*h);
	m *= 1.79284291400159f - 0.85373472095314f * (a0*a0+h*h);
	// Compute final noise value at P
	glm::vec3 g(a0.x  * x0.x  + h.x  * x0.y,a0.y * x1.x + h.y * x1.y,a0.z * x2.x + h.z * x2.y );
	return 130.0 * dot(m, g);
}

static float octaveNoise(const glm::vec2 &x,float persistence = .5, int octaves = 8)
{
	float total = 0.0;
	float frequency = 1.0;
	float amplitude = 1.0;
	float maxValue = 0.0;
	for (int i = 0; i < octaves; i++)
	{
		total += snoise(x * frequency) * amplitude;
		maxValue += amplitude;
		amplitude *= persistence;
		frequency *= 2.0;
	}
	return maxValue != 0.0 ? total / maxValue : 0.0;
}




struct Timer
{
    Timer():startTime(std::chrono::high_resolution_clock::now())
    {}  
    void reset()
    {
      startTime = std::chrono::high_resolution_clock::now();
    }   
    double deltaTime()
    {
      auto endTime = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> duration(endTime-startTime);  // @suppress("Invalid arguments")
      return duration.count();
    }   
    double deltaTimeMilli()
    {
      auto endTime = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime);
      return duration.count();
    }
    private:
    std::chrono::high_resolution_clock::time_point startTime;
};

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

static tga::Shader loadShader(tga::TGAVulkan &tgav, const std::string& filename, tga::ShaderType type)
{
    auto shaderData = readFile(filename);
    return tgav.createShader({type,(uint8_t*)shaderData.data(),shaderData.size()});
}