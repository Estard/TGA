#include "tga/tga.hpp"
#include "tga/tga_utils.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

int main()
{
    // Open the interface
    std::shared_ptr<tga::Interface> tgai = std::make_shared<tga::TGAVulkan>();
    auto [screenResX, screenResY] = tgai->screenResolution();

    // Write into multiple textures
    tga::Shader vertexShaderBG =
        tga::loadShader("shaders/deferred_rendering_bg_vert.spv", tga::ShaderType::vertex, tgai);
    tga::Shader fragmentShaderBG =
        tga::loadShader("shaders/deferred_rendering_bg_frag.spv", tga::ShaderType::fragment, tgai);

    // Textures being written into
    tga::Texture cc1 = tgai->createTexture({screenResX, screenResY, tga::Format::r16g16b16a16_sfloat, nullptr, 0});
    // Can mix and match formats but not resolutions
    tga::Texture cc2 = tgai->createTexture({screenResX, screenResY, tga::Format::r8g8b8a8_srgb, nullptr, 0});
    tga::Texture cc3 = tgai->createTexture({screenResX, screenResY, tga::Format::r32_sfloat, nullptr, 0});
    tga::Texture cc4 = tgai->createTexture({screenResX, screenResY, tga::Format::r16_sfloat, nullptr, 0});
    // Renderpass using the shaders and rendering to the window
    tga::RenderPass renderPassBG = tgai->createRenderPass({{vertexShaderBG, fragmentShaderBG}, {cc1, cc2, cc3, cc4}});

    // Perform operations using output from previous pass as input
    tga::Shader vertexShaderFG, fragmentShaderFG;
    try {
        vertexShaderFG = tga::loadShader("shaders/deferred_rendering_fg_vert.spv", tga::ShaderType::vertex, tgai);
        fragmentShaderFG = tga::loadShader("shaders/deferred_rendering_fg_frag.spv", tga::ShaderType::fragment, tgai);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is your working directory `examples/`?\n";
    }

    tga::Window window = tgai->createWindow({screenResX, screenResY});
    tga::RenderPassInfo rpFGInfo{{vertexShaderFG, fragmentShaderFG}, window};

    // Add a set layout to create binding points for input textures
    rpFGInfo.inputLayout.setLayouts.push_back({{{tga::BindingType::sampler, 1} /*cc1*/,
                                                {tga::BindingType::sampler, 1} /*cc2*/,
                                                {tga::BindingType::sampler, 1} /*cc3*/,
                                                {tga::BindingType::sampler, 1} /*cc4*/}});
    tga::RenderPass renderPassFG = tgai->createRenderPass(rpFGInfo);
    tga::InputSet ccIS = tgai->createInputSet({renderPassFG, 0, {{cc1, 0}, {cc2, 1}, {cc3, 2}, {cc4, 3}}});

    // Single CommandBuffer that will be reused every frame
    tga::CommandBuffer cmdBuffer;

    while (!tgai->windowShouldClose(window)) {
        // Open the CommandBuffer for recording
        tgai->beginCommandBuffer(cmdBuffer);

        // Renders into two textures at the same time
        tgai->setRenderPass(renderPassBG, 0);
        tgai->draw(3, 0);

        // Reads from textures written by first pass and combines them into one
        tgai->setRenderPass(renderPassFG, tgai->nextFrame(window));
        tgai->bindInputSet(ccIS);
        tgai->draw(3, 0);

        cmdBuffer = tgai->endCommandBuffer();

        // Execute commands and show the result
        tgai->execute(cmdBuffer);
        // You should see a white screen, meaning all 4 values have been read and applied
        tgai->present(window);
    }
    return 0;
}