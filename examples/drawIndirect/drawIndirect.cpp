#include "tga/tga.hpp"
#include "tga/tga_math.hpp"
#include "tga/tga_utils.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"

int main()
{
    tga::Interface tgai{};

    // Use utility function to load shaders from file
    tga::Shader vertexShader = tga::loadShader("../shaders/drawIndirect_vert.spv", tga::ShaderType::vertex, tgai);
    tga::Shader fragmentShader = tga::loadShader("../shaders/drawIndirect_frag.spv", tga::ShaderType::fragment, tgai);

    auto [screenResX, screenResY] = tgai.screenResolution();
    tga::Window window = tgai.createWindow({screenResX, screenResY});

    std::vector<tga::DrawIndirectCommand> indirectCommands{/*Draw red triangle*/
                                                           {/*vertexCount=*/3,
                                                            /*instanceCount=*/1,
                                                            /*firstVertex=*/0,
                                                            /*firstInstance=*/0},
                                                           /*Draw green triangle*/
                                                           {3, 1, 3, 0},
                                                           /*Draw blue rectangle*/
                                                           {6, 1, 6, 0}};
    // Create buffer storing the indirect draw commands

    auto stagingData = tgai.createStagingBuffer(
        {indirectCommands.size() * sizeof(tga::DrawIndirectCommand), tga::memoryAccess(indirectCommands)});

    tga::Buffer indirectDrawBuffer = tgai.createBuffer(
        {tga::BufferUsage::indirect, indirectCommands.size() * sizeof(tga::DrawIndirectCommand), stagingData});

    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;
    };
    glm::vec3 red{1, 0.1, 0.1};
    glm::vec3 green{0.1, 1, 0.1};
    glm::vec3 blue{0.1, 0.1, 1.0};
    // vertex buffer storing 3 pieces of geometry
    std::vector<Vertex> vertexBufferCPU{/*red triangle, top left*/
                                        {{-1, -1}, red},
                                        {{-0.5, -1}, red},
                                        {{-0.75, -0.5}, red},
                                        /*green triangle, bottom right*/
                                        {{1, 1}, green},
                                        {{0.75, 0.5}, green},
                                        {{0.5, 1}, green},
                                        /*blue rectangle, center*/
                                        {{-.25, -.25}, blue},
                                        {{-.25, .25}, blue},
                                        {{.25, -.25}, blue},
                                        {{.25, -.25}, blue},
                                        {{-.25, .25}, blue},
                                        {{.25, .25}, blue}};

    tga::VertexLayout vertexLayout(sizeof(Vertex), {{offsetof(Vertex, position), tga::Format::r32g32_sfloat},
                                                    {offsetof(Vertex, color), tga::Format::r32g32b32_sfloat}});

    auto vertexStaging =
        tgai.createStagingBuffer({vertexBufferCPU.size() * sizeof(Vertex), tga::memoryAccess(vertexBufferCPU)});
    tga::Buffer vertexBuffer =
        tgai.createBuffer({tga::BufferUsage::vertex, vertexBufferCPU.size() * sizeof(Vertex), vertexStaging});

    auto rpInfo = tga::RenderPassInfo{vertexShader, fragmentShader, window}.setVertexLayout(vertexLayout);

    tga::RenderPass renderPass = tgai.createRenderPass(rpInfo);

    tga::CommandBuffer cmdBuffer;

    while (!tgai.windowShouldClose(window)) {
        auto nextFrame = tgai.nextFrame(window);
        cmdBuffer = tga::CommandRecorder{tgai, cmdBuffer}
                        .setRenderPass(renderPass, nextFrame)
                        .bindVertexBuffer(vertexBuffer)
                        .drawIndirect(indirectDrawBuffer, indirectCommands.size())
                        .endRecording();

        tgai.execute(cmdBuffer);
        tgai.present(window, nextFrame);
    }
    return 0;
}