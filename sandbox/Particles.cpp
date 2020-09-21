#include "Framework.hpp"
#include "tga/tga_math.hpp"

struct Particle{
    alignas(16) glm::vec4 position; //position.w -> scale of particle;
    alignas(16) glm::vec4 color; 
    alignas(16) uint32_t index; //Particles are reordered, so keep the original index around
};

struct Camera{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

constexpr uint32_t PARTICLE_COUNT = 500;

class Particles : public Framework
{
    std::array<Particle,PARTICLE_COUNT> particles;
    Camera camera{};
    float camSpeed = 10;
    glm::vec3 camPos = {0,-63, -33};
    glm::vec3 lookAt = {0,0,55};
    std::vector<tga::CommandBuffer> cmdBuffers;
    tga::Buffer camBuffer;
    tga::Buffer pBuffer;
    tga::Shader vertexShader,fragmentShader;
    tga::RenderPass renderPass;
    tga::InputSet inputSet;
    
    void OnCreate()
    {
        initParticles();
        pBuffer = tgai->createBuffer({tga::BufferUsage::storage,(uint8_t*)particles.data(),particles.size()*sizeof(Particle)});
        camBuffer = tgai->createBuffer({tga::BufferUsage::uniform,(uint8_t*)&camera,sizeof(camera)});
        vertexShader = loadShader("shaders/particlesVert.spv",tga::ShaderType::vertex);
        fragmentShader = loadShader("shaders/particlesFrag.spv",tga::ShaderType::fragment);

        tga::SetLayout setLayout({{tga::BindingType::uniformBuffer},{tga::BindingType::storageBuffer}});
        tga::RenderPassInfo rpInfo({{vertexShader,fragmentShader},_frameworkWindow, tga::ClearOperation::all});
        rpInfo.inputLayout.setLayouts.push_back(setLayout);
        rpInfo.rasterizerConfig.frontFace = tga::FrontFace::counterclockwise;
        rpInfo.rasterizerConfig.blendEnabled = true;
        renderPass = tgai->createRenderPass(rpInfo);
        inputSet = tgai->createInputSet({renderPass,0,{tga::Binding(camBuffer,0),tga::Binding(pBuffer,1)}});
        
        auto cmdCount = tgai->backbufferCount(_frameworkWindow);
        for(uint32_t i = 0; i < cmdCount; i++){
            tgai->beginCommandBuffer();
            tgai->setRenderPass(renderPass,i);
            tgai->bindInputSet(inputSet);
            tgai->draw(particles.size()*6,0);
            cmdBuffers.emplace_back(tgai->endCommandBuffer());
        }
    }
    void OnUpdate(uint32_t nextFrame)
    {
        static float totalTime = 0.0;
        totalTime += deltaTime;
        updateCam();

        //Update Positions
        for(auto &p : particles){
            auto i = p.index;
            auto s = i%4+1;
            p.position = glm::vec4(s*s*std::sin(i+totalTime),s*s*std::cos(i+totalTime),i,(6-s)*.25);
        }

        //Sort by Distance to Camera
        std::sort(particles.begin(),particles.end(),[&](const Particle &a,const Particle &b)
        {
            return glm::distance2(glm::vec3(a.position.x,a.position.y,a.position.z),camPos) > 
                glm::distance2(glm::vec3(b.position.x,b.position.y,b.position.z),camPos);
        });

        //Update Buffers on GPU
        tgai->updateBuffer(pBuffer,(uint8_t*)particles.data(),particles.size()*sizeof(Particle),0);
        tgai->updateBuffer(camBuffer,(uint8_t*)&camera,sizeof(camera),0);

        //Draw
        tgai->execute(cmdBuffers[nextFrame]);
        std::cout << "Next Frame: "<< nextFrame << " dt: "<<deltaTime<<'\n';
    }


    private:

    void initParticles()
    {
        const std::array<glm::vec4,4> colorPalette{
            glm::vec4(1,0,0,.75),
            glm::vec4(0,1,0,.75),
            glm::vec4(0,0,1,.75),
            glm::vec4(1,1,0,.75)
        };
        for(uint32_t i = 0; i < PARTICLE_COUNT; i++){
            auto s = i%4+1;
            particles[i] = {glm::vec4(s*s*std::sin(i),s*s*std::cos(i),i,(6-s)*.25),colorPalette[i%colorPalette.size()],i};
        }
    }
    tga::Shader loadShader(const std::string& filename,tga::ShaderType type) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> shaderData(fileSize);
        file.seekg(0);
        file.read(shaderData.data(), fileSize);
        file.close();
        return tgai->createShader({type,(uint8_t*)shaderData.data(),shaderData.size()});
    }

    void updateCam()
    {
        

        if(tgai->keyDown(_frameworkWindow,tga::Key::Up))
            lookAt.z += deltaTime*camSpeed;
        if(tgai->keyDown(_frameworkWindow,tga::Key::Down))
            lookAt.z -= deltaTime*camSpeed;
        if(tgai->keyDown(_frameworkWindow,tga::Key::W))
            camPos += glm::normalize(lookAt-camPos)*camSpeed*float(deltaTime);
        if(tgai->keyDown(_frameworkWindow,tga::Key::S))
            camPos -= glm::normalize(lookAt-camPos)*camSpeed*float(deltaTime);
        

        camera.view = glm::lookAt(camPos,lookAt,glm::vec3(0,0,1));
        camera.projection = glm::perspective(glm::radians(60.f),_frameworkWindowWidth/float(_frameworkWindowHeight),0.1f,5000.f);
        camera.projection[1][1] *= -1;

        std::fprintf(stderr,"Campos: {%f\t%f\t%f}\nLookAt: {%f\t%f\t%f}\n",camPos.x,camPos.y,camPos.z,lookAt.x,lookAt.y,lookAt.z);
    }
    
};  

int main()
{
    Particles f;
    try{
        f.run();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    
    
    std::cout << "DONE\n";
}