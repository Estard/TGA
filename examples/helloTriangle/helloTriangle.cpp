#include "tga/tga.hpp"
#include "tga/tga_utils.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

int main()
{
    // Open the interface
    tga::Interface tgai{};

    // Use utility function to load Shader from File
    tga::Shader vertexShader = tga::loadShader("../shaders/triangle_vert.spv", tga::ShaderType::vertex, tgai);
    tga::Shader fragmentShader = tga::loadShader("../shaders/triangle_frag.spv", tga::ShaderType::fragment, tgai);

    // Get screen Resolution, use structured bindings to unpack std::pair
    auto [screenResX, screenResY] = tgai.screenResolution();

    // Window with the size of the screen
    tga::Window window = tgai.createWindow({screenResX * 3 / 4, screenResY * 3 / 4});
    tgai.setWindowTitle(window, "TGA Vulkan Hello Triangle");

    // Renderpass using the shaders and rendering to the window
    tga::RenderPass renderPass = tgai.createRenderPass(
        tga::RenderPassInfo{vertexShader, fragmentShader, window}.setClearOperations(tga::ClearOperation::color));

    // Single CommandBuffer that will be reused every frame
    tga::CommandBuffer cmdBuffer{};

    while (!tgai.windowShouldClose(window)) {
        auto nextFrame = tgai.nextFrame(window);
        cmdBuffer = tga::CommandRecorder{tgai, cmdBuffer}
                        .setRenderPass(renderPass, nextFrame, {0.13, 0.13, 0.13, 1.})
                        .draw(3, 0)
                        .endRecording();

        // Execute commands and show the result
        tgai.execute(cmdBuffer);
        tgai.present(window, nextFrame);
    }
    return 0;
}