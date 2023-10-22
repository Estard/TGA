#include "HeightmapViewer.hpp"

#include <chrono>

#include "shaders.hpp"

HeightmapViewer::HeightmapViewer() : tgai(), heightmapInfo(0, 0, tga::Format::r32_sfloat) {}
void HeightmapViewer::setHeightmap(float *data, uint32_t imageWidth, uint32_t imageHeight, float terrainWidth = 1,
                                   float terrainDepth = 1, float terrainHeight = 1)
{
    if (heightmap) tgai.free(heightmap);

    auto texData = tgai.createStagingBuffer({imageWidth * imageHeight * sizeof(float), (uint8_t *)data});
    heightmapInfo = {imageWidth, imageHeight, tga::Format::r32_sfloat, tga::SamplerMode::linear,
                     tga::AddressMode::clampEdge};
    heightmapInfo.setSrcData(texData);
    heightmap = tgai.createTexture(heightmapInfo);
    tgai.free(texData);
    tData.terrainScale = glm::vec3(terrainWidth, terrainHeight, terrainDepth);
}

void setTextureContent(tga::Texture& target, tga::Interface& tgai, uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    if (target) tgai.free(target);
    auto texData = tgai.createStagingBuffer({4 * width * height, rgba_data});
    target = tgai.createTexture(
        tga::TextureInfo{width, height, tga::Format::r8g8b8a8_srgb, tga::SamplerMode::linear, tga::AddressMode::repeat}
            .setSrcData(texData));
    tgai.free(texData);
}

void HeightmapViewer::setSurfaceLowTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    setTextureContent(grassTex, tgai, rgba_data, width, height);
}
void HeightmapViewer::setSurfaceHighTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    setTextureContent(snowTex, tgai, rgba_data, width, height);
}
void HeightmapViewer::setSideLowTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    setTextureContent(dirtTex, tgai, rgba_data, width, height);
}
void HeightmapViewer::setSideHighTexture(uint8_t *rgba_data, uint32_t width, uint32_t height)
{
    setTextureContent(rockTex, tgai, rgba_data, width, height);
}

void HeightmapViewer::setTextureTiling(float uTiling, float vTiling)
{
    tData.textureTiling = glm::vec2(uTiling, vTiling);
}

void HeightmapViewer::createRescources()
{
    // Create a window
    auto [wx, wy] = tgai.screenResolution();
    window = tgai.createWindow({wx*9/10, wy*9/10, tga::PresentMode::vsync});

    auto hmWidth = heightmapInfo.width;
    auto hmHeight = heightmapInfo.height;

    glm::vec3 startPosition{(tData.terrainScale.x * hmWidth) / 2.f, tData.terrainScale.y / 2.f,
                            (tData.terrainScale.z * hmHeight) / 2.f};

    camController = std::make_unique<CameraController>(tgai, window, 90, wx / float(wy), 0.1f, 30000.f, startPosition,
                                                       glm::vec3{0, 0, 1}, glm::vec3{0, 1, 0});

    camController->speed = 7 * (glm::min(tData.terrainScale.x, tData.terrainScale.z) / 2);
    camController->speedBoost = glm::min(glm::max(tData.terrainScale.x, tData.terrainScale.z) * 2, 12.f);

    // Create the index-buffer
    idxBuffer = tgai.createBuffer(
        {tga::BufferUsage::index | tga::BufferUsage::storage, ((hmWidth - 1) * (hmHeight - 1) * 6) * sizeof(uint32_t)});

    struct {
        uint32_t x, y;
    } comp{hmWidth, hmHeight};
    auto compStaging = tgai.createStagingBuffer({sizeof(comp), reinterpret_cast<uint8_t *>(std::addressof(comp))});
    auto compData = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(comp), compStaging});
    auto indexCS = tgai.createShader({tga::ShaderType::compute, indexSpvCS.data(), indexSpvCS.size()});

    tga::ComputePassInfo indexCPInfo{indexCS};
    indexCPInfo.inputLayout.push_back({{{tga::BindingType::uniformBuffer}, {tga::BindingType::storageBuffer}}});

    auto idxPass = tgai.createComputePass(indexCPInfo);
    auto inputSet = tgai.createInputSet({idxPass, {{compData, 0}, {idxBuffer, 1}}, 0});

    auto cmd = tga::CommandRecorder{tgai}
                   .setComputePass(idxPass)
                   .bindInputSet(inputSet)
                   .dispatch(hmWidth - 1, hmHeight - 1, 1)
                   .endRecording();
    tgai.execute(cmd);
    tgai.waitForCompletion(cmd);
    tgai.free(cmd);
    tgai.free(compStaging);
    tgai.free(inputSet);
    tgai.free(idxPass);
    tgai.free(compData);
    tgai.free(indexCS);

    camDataUB = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(CamData), camController->Data()});
    camMetaDataUB = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(CamMetaData), camController->MetaData()});

    wData.front = glm::vec3(0, 0, 1);
    wData.right = glm::vec3(1, 0, 0);
    wData.up = glm::vec3(0, 1, 0);
    glm::vec3 rotation = {glm::radians(-45.f), glm::radians(0.f), glm::radians(-45 / 2.f)};
    wData.light = glm::vec4(glm::mat3(glm::quat(rotation)) * glm::vec3(0, 0, 1), 1.0);

    //std::printf("LightDir: %1.2f %1.2f %1.2f\n", wData.light.x, wData.light.y, wData.light.z);
    wData.resolution = glm::vec2(wx, wy);
    tData.gridPoints = glm::ivec2(hmWidth, hmHeight);

    auto tDataStaging = tgai.createStagingBuffer({sizeof(tData), (uint8_t *)&tData});
    auto wDataStaging = tgai.createStagingBuffer({sizeof(wData), (uint8_t *)&wData});
    tDataUB = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(tData), tDataStaging});
    wDataUB = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(wData), wDataStaging});

    terrainVS = tgai.createShader({tga::ShaderType::vertex, terrainSpvVS.data(), terrainSpvVS.size()});
    terrainFS = tgai.createShader({tga::ShaderType::fragment, terrainSpvFS.data(), terrainSpvFS.size()});

    tga::InputLayout terrainInputLayout{{// CamData, CamMetaData
                                         {{tga::BindingType::uniformBuffer}, {tga::BindingType::uniformBuffer}},
                                         // TerrainData, WorldData
                                         {{tga::BindingType::uniformBuffer}, {tga::BindingType::uniformBuffer}},
                                         {{tga::BindingType::sampler},
                                          {tga::BindingType::sampler},
                                          {tga::BindingType::sampler},
                                          {tga::BindingType::sampler},
                                          {tga::BindingType::sampler}}}};
    terrainPass = tgai.createRenderPass({terrainVS,
                                         terrainFS,
                                         window,
                                         {},
                                         terrainInputLayout,
                                         tga::ClearOperation::none,
                                         {tga::CompareOperation::less},
                                         {tga::FrontFace::clockwise, tga::CullMode::back}});

    auto texStagingData = tgai.createStagingBuffer({4 * 4 * sizeof(uint8_t)});

    constexpr uint8_t texData[4 * 4] = {0x56, 0x7d, 0x46, 150, 0x9b, 0x76, 0x53, 150,
                                        0x5a, 0x4d, 0x41, 150, 0xfa, 0xfa, 0xfb, 200};

    std::memcpy(tgai.getMapping(texStagingData), texData, sizeof(texData));

    tga::TextureInfo texCreateInfo{1, 1, tga::Format::r8g8b8a8_srgb, tga::SamplerMode::linear,
                                   tga::AddressMode::repeat};
    texCreateInfo.setSrcData(texStagingData);

    if (!grassTex) grassTex = tgai.createTexture(texCreateInfo.setSrcDataOffset(4 * 0));
    if (!dirtTex) dirtTex = tgai.createTexture(texCreateInfo.setSrcDataOffset(4 * 1));
    if (!rockTex) rockTex = tgai.createTexture(texCreateInfo.setSrcDataOffset(4 * 2));
    if (!snowTex) snowTex = tgai.createTexture(texCreateInfo.setSrcDataOffset(4 * 3));

    tgai.free(texStagingData);

    camIS = tgai.createInputSet({terrainPass, {{camDataUB, 0}, {camMetaDataUB, 1}}, 0});
    terrainWorldIS = tgai.createInputSet({terrainPass, {{tDataUB, 0}, {wDataUB, 1}}, 1});
    textureIS = tgai.createInputSet(
        {terrainPass, {{heightmap, 0}, {grassTex, 1}, {dirtTex, 2}, {rockTex, 3}, {snowTex, 4}}, 2});

    auto ppVS = tgai.createShader({tga::ShaderType::vertex, skySpvVert.data(), skySpvVert.size()});
    auto ppFS = tgai.createShader({tga::ShaderType::fragment, skySpvFrag.data(), skySpvFrag.size()});
    skyPass = tgai.createRenderPass({ppVS,
                                     ppFS,
                                     window,
                                     {},
                                     terrainInputLayout,
                                     tga::ClearOperation::all,
                                     {tga::CompareOperation::ignore},
                                     {tga::FrontFace::counterclockwise, tga::CullMode::front}});
    tgai.free(ppVS);
    tgai.free(ppFS);
}

void HeightmapViewer::view()
{
    createRescources();
    tga::CommandBuffer cmdBuffer;

    double deltaTime = 1. / 60.;

    double smoothedDeltaTime = 0;
    size_t deltaTimeCount = 0;

    while (!tgai.windowShouldClose(window)) {
        auto ts = std::chrono::steady_clock::now();

        auto nf = tgai.nextFrame(window);
        auto idxCount = 6 * (heightmapInfo.width - 1) * (heightmapInfo.height - 1);

        cmdBuffer = tga::CommandRecorder{tgai, cmdBuffer}
                        .bufferUpload(camController->Data(), camDataUB, sizeof(CamData))
                        .bufferUpload(camController->MetaData(), camMetaDataUB, sizeof(CamMetaData))
                        .barrier(tga::PipelineStage::Transfer,tga::PipelineStage::VertexShader)
                        .setRenderPass(skyPass,nf)
                        .bindInputSet(camIS)
                        .bindInputSet(textureIS)
                        .bindInputSet(terrainWorldIS)
                        .draw(3,0)
                        .bindIndexBuffer(idxBuffer)
                        .setRenderPass(terrainPass,nf)
                        .drawIndexed(idxCount,0,0)
                        .endRecording();

        camController->update(deltaTime);

        tgai.execute(cmdBuffer);
        tgai.present(window, nf);
        auto tn = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<double>(tn - ts).count();

        smoothedDeltaTime += deltaTime;
        deltaTimeCount++;

        if(smoothedDeltaTime >= 1.0){
            auto fps = deltaTimeCount / smoothedDeltaTime;
            smoothedDeltaTime = 0;
            deltaTimeCount = 0;
            std::stringstream windowTitle;
            windowTitle << "TGA Heightmap Demo (" << fps << " fps)";
            tgai.setWindowTitle(window,windowTitle.str());
        }
    }
}