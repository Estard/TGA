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

int main(int argc, char** argv)
{
    UBO ubo {0.0};
    auto view = glm::lookAt(glm::vec3(2.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280 / (float) 720, 0.1f, 10.0f);
    projection[1][1] *=-1.;
    glm::mat4 viewProjection = projection*view;

    for(auto &vertex:vertices)
        vertex.pos = viewProjection*vertex.pos;

    std::cout << "Executing:\n";
    try
    {
        tga::TGAVulkan tgav = tga::TGAVulkan();
        auto b1 = tgav.createBuffer({tga::BufferUsage::index,(uint8_t*)indices.data(),indices.size()*sizeof(uint32_t)});
        auto b2 = tgav.createBuffer({tga::BufferUsage::vertex,(uint8_t*)vertices.data(),vertices.size()*sizeof(Vertex)});
        auto b3 = tgav.createBuffer({tga::BufferUsage::uniform,(uint8_t*)(&ubo),sizeof(ubo)});
       // std::cout << "Buffer: "<<b1<<' '<<b2<<'\n';
        auto t1 = tgav.createTexture({4,4,(uint8_t*)texture.data(),texture.size()*sizeof(Pixel),tga::Format::r8g8b8a8_unorm,tga::SamplerMode::nearest,tga::RepeatMode::repeate});
       // std::cout <<"Texture: "<< t1<<'\n';

        tga::VertexLayout vertexLayout{sizeof(Vertex),{
            {offsetof(Vertex,pos),tga::Format::r32g32b32a32_sfloat},
            {offsetof(Vertex,uv),tga::Format::r32g32_sfloat}}};
        tga::InputLayout inputLayout{{{{{tga::BindingType::uniformBuffer,1},{tga::BindingType::sampler2D,1}}}}};

        if(argc>2){
            auto shaderCode1 = readFile(argv[1]);
            auto shaderCode2 = readFile(argv[2]);
            auto s1 = tgav.createShader({tga::ShaderType::vertex,(uint8_t*)shaderCode1.data(),shaderCode1.size()});
            auto s2 = tgav.createShader({tga::ShaderType::fragment,(uint8_t*)shaderCode2.data(),shaderCode2.size()});
            std::cout <<"Shader: "<<s1<<' '<<s2<<'\n';
           
            tga::Window w1 = tgav.createWindow({1920,1080});
            std::cout <<"Window: "<<w1<<'\n';

            tga::RenderPassInfo renderPassInfo{{s1,s2},w1};
            renderPassInfo.rasterizerConfig.depthCompareOp = tga::CompareOperation::lessEqual;
            renderPassInfo.rasterizerConfig.cullMode = tga::CullMode::back;
            renderPassInfo.rasterizerConfig.frontFace = tga::FrontFace::counterclockwise;
            renderPassInfo.vertexLayout = vertexLayout;
            renderPassInfo.clearOperations = tga::ClearOperation::all;
            renderPassInfo.inputLayout = inputLayout;

            //auto r1 = tgav.createRenderPass({{s1,s2},t1});
            auto r2 = tgav.createRenderPass(renderPassInfo);
            //std::cout <<"RenderPass: "<< r1 <<' '<<r2<<'\n';
            tga::Binding uboBinding{b3,0,0};
            tga::Binding texBinding{t1,1,0};
            auto is1 = tgav.createInputSet({r2,0,{uboBinding,texBinding}});

            std::vector<tga::CommandBuffer> cmdBuffers{};
            for(uint32_t i = 0; i < tgav.backbufferCount(w1) ; i++)
            {
                tgav.beginCommandBuffer({});
                tgav.bindVertexBuffer(b2);
                tgav.bindIndexBuffer(b1);
                tgav.setRenderPass(r2,i);
                tgav.bindInputSet(is1);
                tgav.drawIndexed(indices.size(),0,0);
                cmdBuffers.push_back(tgav.endCommandBuffer());
            }
            using namespace std::chrono_literals;
            Timer t;
            int accu = 0;
            double timeAccu = 0;
            int accuLimit = 16;
            bool shouldClose{false};
            while(!tgav.windowShouldClose(w1) && !shouldClose)
            {
         //       std::this_thread::sleep_for(7.0ms);
                ubo.time += timeAccu;
                tgav.updateBuffer(b3,(uint8_t*)(&ubo),sizeof(ubo),0);

                auto frame = tgav.nextFrame(w1);

                shouldClose = tgav.keyDown(w1,tga::Key::W) && tgav.keyDown(w1,tga::Key::Control_Left);
                tgav.execute(cmdBuffers[frame]);
                tgav.present(w1);

                double dt = t.deltaTime();
                timeAccu+=dt;
                accu++;

                if(accu == accuLimit){
                    accu = 0;
                    double fps = 1./(timeAccu/accuLimit);
                    timeAccu = 0;
                    std::stringstream ss;
                    ss <<"TGA Vulkan\t"<< (fps)<<" fps";
                    tgav.setWindowTitel(w1,ss.str());
                }
                t.reset();
            }
        }
        
    }
    catch(const std::exception& e)
    {
        std::cerr<<"[ERROR]: "<< e.what() << '\n';
    }
    
    
    std::cout << "Done"<<std::endl;
    return 0;
}