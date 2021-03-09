#include "HeightmapViewer.hpp"
#include "shaders.hpp"
#include <chrono>





HeightmapViewer::HeightmapViewer(std::shared_ptr<tga::Interface> _tgai):
    tgai(_tgai),heightmapInfo(0,0,tga::Format::r32_sfloat)
{}
void HeightmapViewer::setHeightmap(float* data, uint32_t imageWidth, uint32_t imageHeight, float terrainWidth = 1, float terrainDepth = 1, float terrainHeight = 1)
{
    if(heightmap)
        tgai->free(heightmap);
    heightmapInfo = {imageWidth,imageHeight,tga::Format::r32_sfloat,(uint8_t*)data,
        imageWidth*imageHeight*sizeof(float), tga::SamplerMode::linear,tga::AddressMode::clampEdge};
    heightmap = tgai->createTexture(heightmapInfo);
    tData.terrainScale = glm::vec3(terrainWidth,terrainHeight, terrainDepth);
}

void HeightmapViewer::setSurfaceLowTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    if(grassTex)
        tgai->free(grassTex);
    grassTex = tgai->createTexture({width,height,tga::Format::r8g8b8a8_srgb,rgba_data,4*width*height,tga::SamplerMode::linear,tga::AddressMode::repeat});
}
void HeightmapViewer::setSurfaceHighTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    if(snowTex)
        tgai->free(snowTex);
    snowTex = tgai->createTexture({width,height,tga::Format::r8g8b8a8_srgb,rgba_data,4*width*height,tga::SamplerMode::linear,tga::AddressMode::repeat});
}
void HeightmapViewer::setSideLowTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    if(dirtTex)
        tgai->free(dirtTex);
    dirtTex = tgai->createTexture({width,height,tga::Format::r8g8b8a8_srgb,rgba_data,4*width*height,tga::SamplerMode::linear,tga::AddressMode::repeat});
}
void HeightmapViewer::setSideHighTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    if(rockTex)
        tgai->free(rockTex);
    rockTex = tgai->createTexture({width,height,tga::Format::r8g8b8a8_srgb,rgba_data,4*width*height,tga::SamplerMode::linear,tga::AddressMode::repeat});
}


void HeightmapViewer::setTextureTiling(float uTiling, float vTiling)
{
    tData.textureTiling = glm::vec2(uTiling,vTiling);
}

void HeightmapViewer::createRescources()
{
    //Create a window
    auto [wx,wy] = tgai->screenResolution();
    window = tgai->createWindow({wx,wy});


    glm::vec3 startPosition{
        (tData.terrainScale.x*heightmapInfo.width)/2.f,
        tData.terrainScale.y/2.f,
        (tData.terrainScale.z*heightmapInfo.height)/2.f
        };

    camController = std::make_unique<CameraController>(tgai,window,90,wx/float(wy),0.1f,30000.f,
        startPosition,glm::vec3{0,0,1},glm::vec3{0,1,0});
    
    camController->speed = 7*(glm::min(tData.terrainScale.x,tData.terrainScale.z)/2);
    camController->speedBoost = glm::min(glm::max(tData.terrainScale.x,tData.terrainScale.z)*2,12.f);

    //Create the index-buffer
    const std::array<uint32_t,2> dim{heightmapInfo.width,heightmapInfo.height};
    std::vector<uint32_t> proxyIdxBuffer{};
    proxyIdxBuffer.resize((dim[0]-1)*(dim[1]-1)*6);
    idxBuffer = tgai->createBuffer({tga::BufferUsage::index|tga::BufferUsage::storage,
        (uint8_t*)proxyIdxBuffer.data(),proxyIdxBuffer.size()*sizeof(uint32_t)});
    auto compData = tgai->createBuffer({tga::BufferUsage::uniform,(uint8_t*)dim.data(),dim.size()*sizeof(uint32_t)});
    auto indexCS = tgai->createShader({tga::ShaderType::compute,indexSpvCS.data(),indexSpvCS.size()});

    tga::RenderPassInfo idxRpInfo{{indexCS},window};
    idxRpInfo.inputLayout.setLayouts.push_back({{{tga::BindingType::uniformBuffer},{tga::BindingType::storageBuffer}}});
    auto idxPass = tgai->createRenderPass(idxRpInfo);
    auto inputSet = tgai->createInputSet({idxPass,0,{{compData,0},{idxBuffer,1}}});
    tgai->beginCommandBuffer();
    tgai->setRenderPass(idxPass,0);
    tgai->bindInputSet(inputSet);
    tgai->dispatch(dim[0]-1,dim[1]-1,1);
    auto idxCmdBuffer = tgai->endCommandBuffer();
    tgai->execute(idxCmdBuffer);
    tgai->free(idxCmdBuffer);tgai->free(inputSet);tgai->free(idxPass);tgai->free(compData);
    tgai->free(indexCS);

    camDataUB = tgai->createBuffer({tga::BufferUsage::uniform,(uint8_t*)(&camController->Data()),sizeof(CamData)});
    camMetaDataUB = tgai->createBuffer({tga::BufferUsage::uniform,(uint8_t*)(&camController->MetaData()),sizeof(CamMetaData)});


    wData.front = glm::vec3(0,0,1);
    wData.right = glm::vec3(1,0,0);
    wData.up = glm::vec3(0,1,0);
    glm::vec3 rotation = {glm::radians(-45.f),glm::radians(0.f),glm::radians(-45/2.f)};
    wData.light = glm::vec4(glm::mat3(glm::quat(rotation))*glm::vec3(0,0,1),1.0);

    std::printf("LightDir: %1.2f %1.2f %1.2f\n",wData.light.x,wData.light.y,wData.light.z);
    wData.resolution = glm::vec2(wx,wy);
    tData.gridPoints = glm::ivec2(dim[0],dim[1]);
    //tData.terrainScale = wData.front+wData.right+wData.up*glm::min(float(dim[0]),float(dim[1]))/8.f;

    //camController->Position() = glm::vec3(tData.terrainScale*glm::vec3(0.5*dim[0],1.05,0.5*dim[1]));

    tDataUB = tgai->createBuffer({tga::BufferUsage::uniform,(uint8_t*)&tData,sizeof(tData)});
    wDataUB = tgai->createBuffer({tga::BufferUsage::uniform,(uint8_t*)&wData,sizeof(wData)});

    //
    terrainVS = tgai->createShader({tga::ShaderType::vertex,terrainSpvVS.data(),terrainSpvVS.size()});
    terrainFS = tgai->createShader({tga::ShaderType::fragment,terrainSpvFS.data(),terrainSpvFS.size()});

    tga::InputLayout terrainInputLayout{{
        //CamData, CamMetaData
        {{tga::BindingType::uniformBuffer},{tga::BindingType::uniformBuffer}},
        //TerrainData, WorldData
        {{tga::BindingType::uniformBuffer},{tga::BindingType::uniformBuffer}},
        {{tga::BindingType::sampler},{tga::BindingType::sampler},{tga::BindingType::sampler},{tga::BindingType::sampler},{tga::BindingType::sampler}}
    }};
    terrainPass = tgai->createRenderPass({{terrainVS,terrainFS},window,tga::ClearOperation::none,
    {tga::FrontFace::clockwise,tga::CullMode::back},{tga::CompareOperation::less},
        terrainInputLayout});

    uint8_t grass[] = {0x56,0x7d,0x46,150};
    uint8_t dirt[] = {0x9b,0x76,0x53,150};
    uint8_t rock[] = {0x5a,0x4d,0x41,150};
    uint8_t snow[] = {0xfa,0xfa,0xfb,200};
    if(!grassTex)
        grassTex = tgai->createTexture({1,1,tga::Format::r8g8b8a8_srgb,grass,4,tga::SamplerMode::linear,tga::AddressMode::repeat});
    if(!dirtTex)
        dirtTex = tgai->createTexture({1,1,tga::Format::r8g8b8a8_srgb,dirt,4,tga::SamplerMode::linear,tga::AddressMode::repeat});
    if(!rockTex)
        rockTex = tgai->createTexture({1,1,tga::Format::r8g8b8a8_srgb,rock,4,tga::SamplerMode::linear,tga::AddressMode::repeat});
    if(!snowTex)
        snowTex = tgai->createTexture({1,1,tga::Format::r8g8b8a8_srgb,snow,4,tga::SamplerMode::linear,tga::AddressMode::repeat});


    camIS = tgai->createInputSet({terrainPass,0,{{camDataUB,0},{camMetaDataUB,1}}});
    terrainWorldIS = tgai->createInputSet({terrainPass,1,{{tDataUB,0},{wDataUB,1}}});
    textureIS = tgai->createInputSet({terrainPass,2,{{heightmap,0},{grassTex,1},{dirtTex,2},{rockTex,3},{snowTex,4}}});


    auto ppVS = tgai->createShader({tga::ShaderType::vertex,skySpvVert.data(),skySpvVert.size()});
    auto ppFS = tgai->createShader({tga::ShaderType::fragment,skySpvFrag.data(),skySpvFrag.size()});
    skyPass = tgai->createRenderPass({{ppVS,ppFS},window,tga::ClearOperation::all,
    {tga::FrontFace::counterclockwise,tga::CullMode::front},{tga::CompareOperation::ignore},
        terrainInputLayout});
    tgai->free(ppVS);tgai->free(ppFS);
    
}

void HeightmapViewer::view()
{
    createRescources();
    tga::CommandBuffer cmdBuffer;

    double deltaTime = 1./60.;

    while(!tgai->windowShouldClose(window))
    {
        auto ts = std::chrono::steady_clock::now();
        camController->update(deltaTime);
        tgai->updateBuffer(camDataUB,(uint8_t*)(&camController->Data()),sizeof(CamData),0);
        tgai->updateBuffer(camMetaDataUB,(uint8_t*)(&camController->MetaData()),sizeof(CamMetaData),0);

        auto nf = tgai->nextFrame(window);
        tgai->beginCommandBuffer(cmdBuffer);
        tgai->setRenderPass(skyPass,nf);
        tgai->bindInputSet(camIS);
        tgai->bindInputSet(textureIS);
        tgai->bindInputSet(terrainWorldIS);
        tgai->draw(3,0);
        tgai->bindIndexBuffer(idxBuffer);
        tgai->setRenderPass(terrainPass,nf);
        auto idxCount = 6*(heightmapInfo.width-1)*(heightmapInfo.height-1);
        tgai->drawIndexed(idxCount,0,0);
        cmdBuffer = tgai->endCommandBuffer();
        tgai->execute(cmdBuffer);
        tgai->present(window);
        
        auto tn = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<double>(tn-ts).count();
        std::printf("%3.1f",1./deltaTime);
        std::cout << std::flush << "\b\b\b\b\b";
    }
    

}