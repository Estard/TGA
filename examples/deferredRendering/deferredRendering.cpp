#include "tga/tga.hpp"
#include "tga/tga_utils.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

int main()
{
    // Open the interface
    tga::Interface tgai{};
    auto [screenResX, screenResY] = tgai.screenResolution();

    // Write into multiple textures
    tga::Shader vertexShaderBG =
        tga::loadShader("../shaders/deferred_rendering_bg_vert.spv", tga::ShaderType::vertex, tgai);
    tga::Shader fragmentShaderBG =
        tga::loadShader("../shaders/deferred_rendering_bg_frag.spv", tga::ShaderType::fragment, tgai);

    // Textures being written into
    tga::Texture cc1 = tgai.createTexture({screenResX, screenResY, tga::Format::r16g16b16a16_sfloat});
    // Can mix and match formats but not resolutions
    tga::Texture cc2 = tgai.createTexture({screenResX, screenResY, tga::Format::r8g8b8a8_srgb});
    tga::Texture cc3 = tgai.createTexture({screenResX, screenResY, tga::Format::r32_sfloat});
    tga::Texture cc4 = tgai.createTexture({screenResX, screenResY, tga::Format::r16_sfloat});
    // Renderpass using the shaders and rendering to the window
    tga::RenderPass renderPassBG =
        tgai.createRenderPass(tga::RenderPassInfo{vertexShaderBG, fragmentShaderBG}
                                  .setRenderTarget(std::vector<tga::Texture>{cc1, cc2, cc3, cc4})
                                  .setClearOperations(tga::ClearOperation::all));

    // Perform operations using output from previous pass as input
    tga::Shader vertexShaderFG, fragmentShaderFG;
    try {
        vertexShaderFG = tga::loadShader("../shaders/deferred_rendering_fg_vert.spv", tga::ShaderType::vertex, tgai);
        fragmentShaderFG =
            tga::loadShader("../shaders/deferred_rendering_fg_frag.spv", tga::ShaderType::fragment, tgai);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cerr << "Is your working directory `examples/`?\n";
    }

    auto setLayout = tga::SetLayout{{tga::BindingType::sampler, 1} /*cc1*/,
                                    {tga::BindingType::sampler, 1} /*cc2*/,
                                    {tga::BindingType::sampler, 1} /*cc3*/,
                                    {tga::BindingType::sampler, 1} /*cc4*/};
    tga::Window window = tgai.createWindow({screenResX, screenResY});
    auto rpFGInfo =
        tga::RenderPassInfo{vertexShaderFG, fragmentShaderFG, window}.setInputLayout(tga::InputLayout{setLayout});

    // Add a set layout to create binding points for input textures
    tga::RenderPass renderPassFG = tgai.createRenderPass(rpFGInfo);
    tga::InputSet ccIS = tgai.createInputSet(
        tga::InputSetInfo(renderPassFG).setBindings({{cc1, 0}, {cc2, 1}, {cc3, 2}, {cc4, 3}}).setIndex(0));

    tga::CommandBuffer cmdBuffer;

    while (!tgai.windowShouldClose(window)) {
        auto nextFrame = tgai.nextFrame(window);
        cmdBuffer = tga::CommandRecorder{tgai, cmdBuffer}
                        .setRenderPass(renderPassBG, 0)
                        .draw(3, 0)
                        .setRenderPass(renderPassFG, nextFrame)
                        .bindInputSet(ccIS)
                        .draw(3, 0)
                        .endRecording();

        tgai.execute(cmdBuffer);

        // You should see a white screen, meaning all 4 values have been read and properly combined
        tgai.present(window, nextFrame);
    }
    return 0;
}