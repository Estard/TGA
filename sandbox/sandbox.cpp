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
glm::vec3 mod289(const glm::vec3 &x) { return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f; }
glm::vec2 mod289(const glm::vec2 &x) { return x - glm::floor(x * (1.0f / 289.0f)) * 289.0f; }
glm::vec3 permute(const glm::vec3 &x) { return mod289(((x*34.0f)+1.0f)*x); }
float snoise(const glm::vec2 &v) {

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

float octaveNoise(const glm::vec2 &x,float persistence = .9, int octaves = 8)
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

struct Vertex{
    glm::vec4 pos;
    glm::vec2 uv;
};


 std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f, 1.f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f, 1.f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f, 1.f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f, 1.f}, {1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f, 1.f}, {1.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f, 1.f}, {0.0f, 1.0f} },
    {{0.5f, 0.5f, -0.5f, 1.f}, {0.0f, 0.0f}  },
    {{-0.5f, 0.5f, -0.5f, 1.f}, {1.0f, 0.0f} }
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

struct UBO{
    float time;
};

struct Pixel{
    uint8_t r,g,b,a;  
};

std::vector<Pixel> texture= {
    {255,1,255,255},{255,255,255,255},{255,255,1,255},{126,126,126,255},
    {255,1,255,255},{255,255,255,255},{255,255,1,255},{126,126,126,255},
    {255,1,255,255},{255,255,255,255},{255,255,1,255},{126,126,126,255},
    {255,1,255,255},{255,255,255,255},{255,255,1,255},{126,126,126,255},
};

class Sandbox{

    tga::TGAVulkan tgav;
    
    public:

    tga::Shader loadShader(const std::string& filename, tga::ShaderType type)
    {
        auto shaderData = readFile(filename);
        return tgav.createShader({type,(uint8_t*)shaderData.data(),shaderData.size()});
    }
    Sandbox():tgav(tga::TGAVulkan())
    {
        vertShader = loadShader("shaders/rectangleVert.spv",tga::ShaderType::vertex);
        fragShader = loadShader("shaders/rectangleFrag.spv",tga::ShaderType::fragment);
        textureShader = loadShader("shaders/textureFrag.spv",tga::ShaderType::fragment);
        uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
        rng.seed(ss);
        unif = std::uniform_real_distribution<double>(0, 1);
        testCompute();
    }

    float noise(float x, float y)
    {  
        return octaveNoise({x,y})*.5+.5; 
    }

    std::vector<float> createHeightmap()
    {
        Timer t;
        uint32_t threads = std::thread::hardware_concurrency();
        threads = threads - threads%4;
        uint32_t workPerThread = (4096*4096) / threads;
        std::vector<float> map(4096*4096);
        std::list<std::future<void>> handles;
        for(uint32_t i = 0; i < threads; i++){
            handles.push_back(std::async([&map](uint32_t start,uint32_t count){
            for(uint32_t e = 0; e < count; e++){
                uint32_t index = start+e;
                uint32_t x = index&4095;
                uint32_t y = (index-x)/4069;
                map[index] = octaveNoise({x/4096.f,y/4096.f})*.5+.5;
            }return;
            },i*workPerThread,workPerThread));
        }
        for(auto &h: handles)
            h.get();
        std::cout <<"Heightmap creation took: "<< t.deltaTime()<<'s'<<std::endl;
        return map;
    }

    void testCompute()
    {
        auto shader = loadShader("shaders/testComputeComp.spv",tga::ShaderType::compute);
        auto proxyTex = tgav.createTexture({1,1,tga::Format::r8_unorm});
        auto rp = tgav.createRenderPass({{shader}, proxyTex});
        tgav.beginCommandBuffer();
        tgav.setRenderPass(rp,0);
        auto cmdBuffer = tgav.endCommandBuffer();
        tgav.execute(cmdBuffer);
    }

    
    void run()
    {   
        auto map = createHeightmap();
        auto renderTex = tgav.createTexture({width,height,tga::Format::r8g8b8a8_unorm,nullptr,width*height*4});
        auto window = tgav.createWindow({width,height,tga::PresentMode::vsync});

        auto heightTex = tgav.createTexture({4096,4096,tga::Format::r32_sfloat,(uint8_t*)map.data(),map.size()*sizeof(map[0]),tga::SamplerMode::linear,tga::RepeatMode::clampBorder});

        auto firstPass = tgav.createRenderPass({{vertShader,fragShader},renderTex});

        tga::RenderPassInfo texturePassInfo({vertShader,textureShader},window);
        texturePassInfo.inputLayout.setLayouts.emplace_back(tga::SetLayout({{tga::BindingType::sampler2D,1}}));
        auto texturePass = tgav.createRenderPass(texturePassInfo);

        tga::Binding texBinding{heightTex,0,0};
        auto inputSet = tgav.createInputSet({texturePass,0,{texBinding}});
       
        std::vector<tga::CommandBuffer> cmdBuffers{};
        for(uint32_t i = 0; i < tgav.backbufferCount(window);i++){
            tgav.beginCommandBuffer();
            tgav.setRenderPass(firstPass,i);
            tgav.draw(3,0);
            tgav.setRenderPass(texturePass,i);
            tgav.bindInputSet(inputSet);
            tgav.draw(3,0);
            cmdBuffers.emplace_back(tgav.endCommandBuffer());
        }

        Timer t;
        double time = 0.;
        uint64_t frames = 0.;
        while(!tgav.windowShouldClose(window)){
            auto nextFrame = tgav.nextFrame(window);
            if(tgav.keyDown(window,tga::Key::MouseLeft)){
                auto [mX,mY] = tgav.mousePosition(window);
                std::cout << "Mouse Position: "<<mX<<' '<<mY<<'\n';
            }
            tgav.execute(cmdBuffers[nextFrame]);
            tgav.present(window);
            std::stringstream ss;

            //Reset when numbers get too big, will take years though
            if(frames > std::numeric_limits<uint32_t>::max()){
                time = 0;
                frames = 0;
            }

            time += t.deltaTime();
            frames++;
            ss <<"TGA Vulkan \t" << (frames/time) << " fps";
            auto title = ss.str();
            tgav.setWindowTitel(window,title);
            t.reset();
        }
    }

    private:
    uint32_t width = 1600;
    uint32_t height = 900;
    tga::Shader vertShader;
    tga::Shader fragShader;
    tga::Shader textureShader;
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> unif;
};

int main(void)
{
    try
    {
        Sandbox sb;
        sb.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Shutdown"<<std::endl;
}