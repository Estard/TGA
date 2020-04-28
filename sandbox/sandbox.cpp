#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

#include <chrono>
#include <sstream>
#include <thread>

#include "tga/tga_math.h"

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
    Sandbox():tgav(tga::TGAVulkan()){}

    tga::Shader loadShader(const std::string& filename, tga::ShaderType type)
    {
        auto shaderData = readFile(filename);
        return tgav.createShader({type,(uint8_t*)shaderData.data(),shaderData.size()});
    }
    void run()
    {
        uint32_t width = 1280;
        uint32_t height = 720;
        auto vertShader = loadShader("shaders/rectangleVert.spv",tga::ShaderType::vertex);
        auto fragShader = loadShader("shaders/rectangleFrag.spv",tga::ShaderType::fragment);
        auto textureShader = loadShader("shaders/textureFrag.spv",tga::ShaderType::fragment);

        auto renderTex = tgav.createTexture({width,height,tga::Format::r8g8b8a8_unorm,nullptr,width*height*4});
        auto window = tgav.createWindow({width,height,tga::PresentMode::vsync});

        auto firstPass = tgav.createRenderPass({{vertShader,fragShader},renderTex});

        tga::RenderPassInfo texturePassInfo({vertShader,textureShader},window);
        texturePassInfo.inputLayout.setLayouts.emplace_back(tga::SetLayout({{tga::BindingType::sampler2D,1}}));
        auto texturePass = tgav.createRenderPass(texturePassInfo);

        tga::Binding texBinding{renderTex,0,0};
        auto inputSet = tgav.createInputSet({texturePass,0,{texBinding}});
       
        std::vector<tga::CommandBuffer> cmdBuffers{};
        for(uint32_t i = 0; i < tgav.backbufferCount(window);i++){
            tgav.beginCommandBuffer({});
            tgav.setRenderPass(firstPass,i);
            tgav.draw(3,0);
            tgav.setRenderPass(texturePass,i);
            tgav.bindInputSet(inputSet);
            tgav.draw(3,0);
            cmdBuffers.emplace_back(tgav.endCommandBuffer());
        }

        while(!tgav.windowShouldClose(window)){
            auto nextFrame = tgav.nextFrame(window);
            if(tgav.keyDown(window,tga::Key::MouseLeft)){
                auto [mX,mY] = tgav.mousePosition(window);
                std::cout << "Mouse Position: "<<mX<<' '<<mY<<'\n';
            }
            tgav.execute(cmdBuffers[nextFrame]);
            tgav.present(window);
        }
    }
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