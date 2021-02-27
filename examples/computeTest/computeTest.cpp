#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

#include <chrono>
#include <sstream>
#include <thread>
#include <random>
#include <future>
#include <list>

#include "tga/tga_math.hpp"

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

class ComputeIndexBuffer{

    uint32_t resX,resY;
    tga::TGAVulkan tgav;
    std::vector<uint32_t> indices;
    public:

    ComputeIndexBuffer(uint32_t _resX, uint32_t _resY):resX(_resX),resY(_resY),tgav(tga::TGAVulkan()),indices((resX-1)*(resY-1)*6,-1)
    {

    }

    void run()
    {
        const std::array<uint32_t,2> dim{resX,resY};
        auto data = tgav.createBuffer({tga::BufferUsage::uniform,(uint8_t*)dim.data(),dim.size()*sizeof(uint32_t)});
        auto buf = tgav.createBuffer({tga::BufferUsage::storage,(uint8_t*)indices.data(),indices.size()*sizeof(uint32_t)});
        auto shader = loadShader(tgav,"shaders/indexComp.spv",tga::ShaderType::compute);
        auto tex = tgav.createTexture({1,1,tga::Format::r32_sfloat});
        tga::RenderPassInfo rpInfo{{shader},tex};
        rpInfo.inputLayout.setLayouts.push_back({{{tga::BindingType::uniformBuffer},{tga::BindingType::storageBuffer}}});
        auto pass = tgav.createRenderPass(rpInfo);
        auto inputSet = tgav.createInputSet({pass,0,{{data,0},{buf,1}}});
        tgav.beginCommandBuffer();
        tgav.setRenderPass(pass,0);
        tgav.bindInputSet(inputSet);
        tgav.dispatch(resX-1,resY-1,1);
        auto cmd = tgav.endCommandBuffer();
        
        tgav.execute(cmd);
        auto res = tgav.readback(buf);
        
        if(res.size()>=(indices.size()*sizeof(uint32_t)))
        {
            std::memcpy(indices.data(),res.data(),indices.size()*sizeof(uint32_t));
            for(size_t i = 0; i < indices.size();i++)
            {
                std::cout << indices[i]<<' ';
                if((i+1)%6==0&&i>0)
                    std::cout <<'\n';
            }
        }
        else
        {
            std::cerr <<"readback wasn't successful\n";
        }
    }
};



int main(void)
{
    try
    {
        ComputeIndexBuffer sb{16,16};
        sb.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Shutdown"<<std::endl;
}