#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"
#include "tga/tga_utils.hpp"

int main()
{
    // Open the interface
    tga::Interface& tgai = std::make_shared<tga::TGAVulkan>();

    // Use utility function to load Shader from File
    tga::Shader vertexShader = tga::loadShader("shaders/triangle_vert.spv",tga::ShaderType::vertex,tgai);
    tga::Shader fragmentShader = tga::loadShader("shaders/triangle_frag.spv",tga::ShaderType::fragment,tgai);

    // Get screen Resolution, use structured bindings to unpack std::pair
    auto [screenResX, screenResY] = tgai.screenResolution();

    // Window with the size of the screen
    tga::Window window = tgai.createWindow({screenResX,screenResY});

    // Renderpass using the shaders and rendering to the window
    tga::RenderPass renderPass = tgai.createRenderPass({{vertexShader,fragmentShader}, window});

    // Single CommandBuffer that will be reused every frame
    tga::CommandBuffer cmdBuffer;

    while(!tgai.windowShouldClose(window)){

        // Open the CommandBuffer for recording
        tgai.beginCommandBuffer(cmdBuffer);

        // Configure the pipeline to render to the next backbuffer
        tgai.setRenderPass(renderPass,tgai.nextFrame(window));

        // The triangle data is procedually drawn in the shader
        tgai.draw(3,0);

        // Accept the filled CommandBuffer
        cmdBuffer = tgai.endCommandBuffer();

        // Execute commands and show the result
        tgai.execute(cmdBuffer);
        tgai.present(window);
    }
    return 0;
}