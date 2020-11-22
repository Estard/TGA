#include "Framework.hpp"


class Derivative : public Framework
{
    std::vector<tga::CommandBuffer> cmdBuffers;
    void OnCreate()
    {
        auto cmdCount = tgai->backbufferCount(_frameworkWindow);
        for(uint32_t i = 0; i < cmdCount; i++){
            tgai->beginCommandBuffer();
            cmdBuffers.emplace_back(tgai->endCommandBuffer());
        }
    }
    void OnUpdate(uint32_t nextFrame)
    {
        tgai->execute(cmdBuffers[nextFrame]);
        std::cout << "Next Frame: "<< nextFrame << " dt: "<<deltaTime<<'\n';
    }
};

int main()
{
    Derivative f;
    try{
        f.run(512,512);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    
    
    std::cout << "DONE\n";
}