#pragma once
#include "CameraController.hpp"
#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

struct TerrainData {
    alignas(16) glm::ivec2 gridPoints = glm::ivec2(1);
    alignas(16) glm::vec3 terrainScale = glm::vec3(1);
    alignas(16) glm::vec2 textureTiling = glm::vec2(1);
};

struct WorldData {
    alignas(16) glm::vec3 front;
    alignas(16) glm::vec3 up;
    alignas(16) glm::vec3 right;
    alignas(16) glm::vec4 light;  // xyz: Direction, w: Intenstity
    alignas(16) glm::vec2 resolution;
    alignas(16) float time;
};

class HeightmapViewer {
public:
    HeightmapViewer();
    void setHeightmap(float *data, uint32_t imageWidth, uint32_t imageHeight, float terrainWidth, float terrainDepth,
                      float terrainHeight);
    void setSurfaceLowTexture(uint8_t *rgba_data, uint32_t width, uint32_t height);
    void setSurfaceHighTexture(uint8_t *rgba_data, uint32_t width, uint32_t height);
    void setSideLowTexture(uint8_t *rgba_data, uint32_t width, uint32_t height);
    void setSideHighTexture(uint8_t *rgba_data, uint32_t width, uint32_t height);
    void setTextureTiling(float uTiling, float vTiling);
    void view();

private:
    void createRescources();

    tga::Interface tgai;
    std::unique_ptr<CameraController> camController;

    tga::Texture heightmap, grassTex, dirtTex, rockTex, snowTex;
    tga::Window window;
    tga::Buffer idxBuffer, camDataUB, camMetaDataUB, tDataUB, wDataUB;
    tga::Shader terrainVS, terrainFS;
    tga::RenderPass terrainPass, skyPass;
    tga::InputSet camIS, terrainWorldIS, textureIS;
    tga::TextureInfo heightmapInfo;

    TerrainData tData;
    WorldData wData;
};