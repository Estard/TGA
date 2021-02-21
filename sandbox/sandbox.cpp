#include "util.hpp"

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

struct Heightmap{
    std::vector<float> data;
    uint32_t width, height;
};



static uint32_t width = 1600;
static uint32_t height = 900;

class TerrainViewer{

    tga::TGAVulkan &tgav;
    tga::Texture heightmap;
    tga::Window window;
    tga::Shader vertShader;
    tga::Shader fragShader;
    struct Data{
        alignas(16) float time = 0.0;
        alignas(16) glm::vec3 scale{4,4,2048};
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    }data;


    std::vector<uint32_t> indexBufferData;
    tga::Buffer indexBuffer;
    glm::vec3 camPos{0,-512,512};
    glm::vec3 camLookAt{0,1,-1};
    public:
    TerrainViewer(tga::TGAVulkan &_tgav, tga::Texture _heightmap, tga::Window _window): 
        tgav(_tgav), heightmap(_heightmap), window(_window)
    {
        vertShader = loadShader(tgav,"shaders/TerrainSampleVert.spv",tga::ShaderType::vertex);
        fragShader = loadShader(tgav,"shaders/TerrainSampleFrag.spv",tga::ShaderType::fragment);
        data.view = glm::lookAt(camPos,glm::vec3(0),glm::vec3(0,0,1));
        data.projection = glm::perspective(glm::radians(60.f),width/float(height),0.1f,5000.f);
        data.projection[1][1] *= -1;

        for(uint32_t y = 0; y < 1024-1; y++)
            for(uint32_t x = 0; x < 1024-1; x++){
                indexBufferData.push_back(x+y*1024);
                indexBufferData.push_back((x+1)+y*1024);
                indexBufferData.push_back(x+(y+1)*1024);

                indexBufferData.push_back(x+(y+1)*1024);
                indexBufferData.push_back((x+1)+y*1024);
                indexBufferData.push_back((x+1)+(y+1)*1024);
            }
        indexBuffer = tgav.createBuffer({tga::BufferUsage::index,(uint8_t*)indexBufferData.data(),
        indexBufferData.size()*sizeof(indexBufferData[0])});
        run();
    }
    private:
    glm::vec2 orientations[4] = {{0,1},{-1,0},{0,-1},{1,0}};
    int oI = 0;
    void moveCam(float dt)
    {

        float hSpeed = 100.;
        float vSpeed = 100.;

        static bool aPressed{false};
        static bool aPrev{false};
        static bool dPressed{false};
        static bool dPrev{false};

        aPressed = tgav.keyDown(window,tga::Key::A);
        if(aPressed && (aPrev!=aPressed))
            oI = (oI+1)%4;
        aPrev = aPressed;

        dPressed = tgav.keyDown(window,tga::Key::D);
        if(dPressed && (dPrev!=dPressed))
            oI = (oI+4-1)%4;
        dPrev = dPressed;

        camLookAt.x = orientations[oI].x;
        camLookAt.y = orientations[oI].y;

        if(tgav.keyDown(window,tga::Key::Control_Left))
            vSpeed *= 2.;

        if(tgav.keyDown(window,tga::Key::Space))
            camPos.z += hSpeed*dt;
        if(tgav.keyDown(window,tga::Key::Shift_Left))
            camPos.z -= hSpeed*dt;
        if(tgav.keyDown(window,tga::Key::W))
            camPos += camLookAt*(dt*vSpeed);
        if(tgav.keyDown(window,tga::Key::S))
            camPos -= camLookAt*(dt*vSpeed);
        if(tgav.keyDown(window,tga::Key::Up))
            camLookAt.z += dt*1.;
        if(tgav.keyDown(window,tga::Key::Down))
            camLookAt.z -= dt*1.;
        
        
    }

    void run()
    {   
        auto ubo = tgav.createBuffer({tga::BufferUsage::uniform,(uint8_t*)&data,sizeof(data)});
        tga::RenderPassInfo rpInfo = {{vertShader,fragShader},window};
        rpInfo.inputLayout.setLayouts.push_back({{{tga::BindingType::uniformBuffer},{tga::BindingType::sampler}}});
        rpInfo.rasterizerConfig.frontFace = tga::FrontFace::counterclockwise;
        rpInfo.perSampleOperations.depthCompareOp = tga::CompareOperation::lessEqual;
        rpInfo.rasterizerConfig.cullMode = tga::CullMode::back;
        //rpInfo.rasterizerConfig.polygonMode = tga::PolygonMode::wireframe;
        rpInfo.clearOperations = tga::ClearOperation::all;
        auto rp = tgav.createRenderPass(rpInfo);
        tga::Binding uboBinding{ubo,0};
        tga::Binding texBinding{heightmap,1};
        auto inputSet = tgav.createInputSet({rp,0,{uboBinding,texBinding}});
        std::vector<tga::CommandBuffer> cmdBuffers{};
        for(uint32_t i = 0; i < tgav.backbufferCount(window);i++){
            tgav.beginCommandBuffer();
            tgav.bindIndexBuffer(indexBuffer);
            tgav.setRenderPass(rp,i);
            tgav.bindInputSet(inputSet);
            tgav.drawIndexed(indexBufferData.size(),0,0);
            cmdBuffers.emplace_back(tgav.endCommandBuffer());
        }
        Timer t;

        while(!tgav.windowShouldClose(window)){
            auto dt = t.deltaTime();
            data.time += dt;
            t.reset();
            moveCam(dt);
            data.view = glm::lookAt(camPos,camPos+camLookAt,glm::vec3(0,0,1));
            tgav.updateBuffer(ubo,(uint8_t*)&data,sizeof(data),0);
            tgav.execute(cmdBuffers[tgav.nextFrame(window)]);
            tgav.present(window);
        }
    }
};

class Sandbox{

    tga::TGAVulkan tgav;
    
    public:
    Sandbox():tgav(tga::TGAVulkan())
    {
        vertShader = loadShader(tgav,"shaders/rectangleVert.spv",tga::ShaderType::vertex);
        fragShader = loadShader(tgav,"shaders/rectangleFrag.spv",tga::ShaderType::fragment);
        textureShader = loadShader(tgav,"shaders/textureFrag.spv",tga::ShaderType::fragment);
        uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
        rng.seed(ss);
        unif = std::uniform_real_distribution<double>(0, 1);
        testCompute();
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
                glm::vec2 st{x/4096.f,y/4096.f};
                map[index] = octaveNoise(st,.5,8)*.5+.5;
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
        auto shader = loadShader(tgav,"shaders/testComputeComp.spv",tga::ShaderType::compute);
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
        auto heightTex = tgav.createTexture({4096,4096,tga::Format::r32_sfloat,(uint8_t*)map.data(),map.size()*sizeof(map[0]),tga::SamplerMode::linear,tga::RepeatMode::clampEdge});
        auto window = tgav.createWindow({width,height,tga::PresentMode::vsync});
        TerrainViewer tv(tgav,heightTex,window);

        auto renderTex = tgav.createTexture({width,height,tga::Format::r8g8b8a8_unorm,nullptr,width*height*4});
        auto firstPass = tgav.createRenderPass({{vertShader,fragShader},renderTex});

        tga::RenderPassInfo texturePassInfo({vertShader,textureShader},window);
        texturePassInfo.inputLayout.setLayouts.emplace_back(tga::SetLayout({{tga::BindingType::sampler,1}}));
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
    tga::Shader vertShader;
    tga::Shader fragShader;
    tga::Shader textureShader;
    std::mt19937_64 rng;
    std::uniform_real_distribution<double> unif;
};

struct GPUMemcopy{

    std::array<float, 32> buffer={};

    void run()
    {
        for(size_t i  = 0; i < buffer.size();i++)
            buffer[i] = float(i);
        tga::TGAVulkan tgav;
        auto buf = tgav.createBuffer({tga::BufferUsage::uniform,(uint8_t*)buffer.data(),buffer.size()*sizeof(buffer[0])});

        for(size_t i  = 0; i < buffer.size();i++)
            buffer[i]++;
        auto readBack = tgav.readback(buf);
        float *readBackFloat = (float*)readBack.data();

        for(size_t i  = 0; i < buffer.size();i++){
            std::cout << buffer[i] <<' ';
        }
        std::cout <<'\n';
        for(size_t i  = 0; i < buffer.size();i++){
            std::cout << readBackFloat[i] <<' ';
        }
        std::cout <<'\n';
        auto tex = tgav.createTexture({4,8,tga::Format::r32_sfloat,(uint8_t*)buffer.data(),buffer.size()*sizeof(buffer[0])});
        readBack = tgav.readback(tex);
        readBackFloat = (float*)readBack.data();   
        for(size_t i  = 0; i < buffer.size();i++){
            std::cout << readBackFloat[i] <<' ';
        }
        std::cout <<'\n';    
    }
};

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