#include <filesystem>
#include <memory_resource>
#include <new>

#include "Framework.hpp"
#include "tga/tga_math.hpp"

struct Particle {
    alignas(16) glm::vec4 position;  // position.w -> scale of particle;
    alignas(16) glm::vec4 color;
    alignas(16) uint32_t index;  // Particles are reordered, so keep the original index around
};

struct Camera {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

static constexpr size_t PARTICLE_COUNT = 500;
static constexpr auto PARTICLE_DATA_SIZE = PARTICLE_COUNT * sizeof(Particle);

class Particles : public Framework {
    tga::StagingBuffer particleStagingBuffer;
    std::pmr::monotonic_buffer_resource particleMemory;
    std::pmr::vector<Particle> particles;

    tga::StagingBuffer cameraStagingBuffer;
    Camera *camera;
    float camSpeed = 10;
    glm::vec3 camPos = {0, -63, -33};
    glm::vec3 lookAt = {0, 0, 55};

    tga::CommandBuffer cmdBuffer;
    tga::Buffer camBuffer;
    tga::Buffer pBuffer;
    tga::Shader vertexShader, fragmentShader;
    tga::RenderPass renderPass;
    tga::InputSet inputSet;

    float smoothedDeltaTime{0};
    uint32_t frameCount{0};

    void OnCreate()
    {
        this->particleStagingBuffer = tgai.createStagingBuffer({PARTICLE_DATA_SIZE});
        auto mapping = tgai.getMapping(particleStagingBuffer);

        // Reconstructing the buffer resource in place because it doesn't allow reassignment
        new (&this->particleMemory)
            std::pmr::monotonic_buffer_resource(mapping, PARTICLE_DATA_SIZE, std::pmr::null_memory_resource());

        // Reconstructing the particles in place because move assignment doesn't propagate the allocator properly
        new (&this->particles) std::pmr::vector<Particle>(&this->particleMemory);
        particles.resize(PARTICLE_COUNT);
        assert(mapping == reinterpret_cast<void *>(particles.data()));
        initParticles();
        this->pBuffer = tgai.createBuffer({tga::BufferUsage::storage, PARTICLE_DATA_SIZE, particleStagingBuffer});

        this->cameraStagingBuffer = tgai.createStagingBuffer({sizeof(Camera)});
        this->camera = static_cast<Camera *>(tgai.getMapping(cameraStagingBuffer));
        camBuffer = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(Camera)});

        vertexShader = loadShader("../shaders/particles_vert.spv", tga::ShaderType::vertex);
        fragmentShader = loadShader("../shaders/particles_frag.spv", tga::ShaderType::fragment);

        tga::SetLayout setLayout({{tga::BindingType::uniformBuffer}, {tga::BindingType::storageBuffer}});
        tga::RenderPassInfo rpInfo({vertexShader, fragmentShader, _frameworkWindow});
        rpInfo.setClearOperations(tga::ClearOperation::all)
            .setInputLayout({setLayout})
            .setRasterizerConfig(tga::RasterizerConfig().setFrontFace(tga::FrontFace::counterclockwise))
            .setPerPixelOperations(tga::PerPixelOperations().setBlendEnabled(true));
        renderPass = tgai.createRenderPass(rpInfo);
        inputSet = tgai.createInputSet({renderPass, {tga::Binding(camBuffer, 0), tga::Binding(pBuffer, 1)}, 0});
    }
    void OnUpdate(uint32_t nextFrame)
    {
        cmdBuffer = tga::CommandRecorder{tgai, cmdBuffer}
                        .bufferUpload(this->cameraStagingBuffer, this->camBuffer, sizeof(Camera))
                        .bufferUpload(this->particleStagingBuffer, pBuffer, PARTICLE_DATA_SIZE)
                        .barrier(tga::PipelineStage::Transfer, tga::PipelineStage::VertexShader)
                        .setRenderPass(renderPass, nextFrame)
                        .bindInputSet(inputSet)
                        .draw(particles.size() * 6, 0)
                        .endRecording();

        static float totalTime = 0.0;
        totalTime += deltaTime;
        updateCam();

        // Update Positions
        for (auto& p : particles) {
            auto i = p.index;
            auto s = i % 4 + 1;
            p.position = glm::vec4(s * s * std::sin(i + totalTime), s * s * std::cos(i + totalTime), i, (6 - s) * .25);
        }

        // Sort by Distance to Camera
        std::sort(particles.begin(), particles.end(), [&](const Particle& a, const Particle& b) {
            return glm::distance2(glm::vec3(a.position.x, a.position.y, a.position.z), camPos) >
                   glm::distance2(glm::vec3(b.position.x, b.position.y, b.position.z), camPos);
        });

        // Draw
        tgai.execute(cmdBuffer);

        this->frameCount++;
        this->smoothedDeltaTime += this->deltaTime;

        if (frameCount < 64) return;
        auto avgFPS = frameCount / (smoothedDeltaTime);
        frameCount = 0;
        smoothedDeltaTime = 0;
        std::stringstream windowTitle;
        windowTitle << "TGA Particle Demo ( " << avgFPS << " fps)";
        tgai.setWindowTitle(this->_frameworkWindow, windowTitle.str());
    }

private:
    void initParticles()
    {
        const std::array<glm::vec4, 4> colorPalette{glm::vec4(1, 0.1, 0.1, .75), glm::vec4(0.1, 1, 0.1, .75),
                                                    glm::vec4(0.1, 0.1, 1, .75), glm::vec4(1, 1, 0.1, .75)};
        for (uint32_t i = 0; i < particles.size(); i++) {
            auto s = i % 4 + 1;
            particles[i] = {glm::vec4(s * s * std::sin(i), s * s * std::cos(i), i, (6 - s) * .25),
                            colorPalette[i % colorPalette.size()], i};
        }
    }
    tga::Shader loadShader(const std::string& filename, tga::ShaderType type)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> shaderData(fileSize);
        file.seekg(0);
        file.read(shaderData.data(), fileSize);
        file.close();
        return tgai.createShader({type, (uint8_t *)shaderData.data(), shaderData.size()});
    }

    void updateCam()
    {
        if (tgai.keyDown(_frameworkWindow, tga::Key::Up)) lookAt.z += deltaTime * camSpeed;
        if (tgai.keyDown(_frameworkWindow, tga::Key::Down)) lookAt.z -= deltaTime * camSpeed;
        if (tgai.keyDown(_frameworkWindow, tga::Key::W))
            camPos += glm::normalize(lookAt - camPos) * camSpeed * float(deltaTime);
        if (tgai.keyDown(_frameworkWindow, tga::Key::S))
            camPos -= glm::normalize(lookAt - camPos) * camSpeed * float(deltaTime);

        camera->view = glm::lookAt(camPos, lookAt, glm::vec3(0, 0, 1));
        camera->projection =
            glm::perspective(glm::radians(60.f), _frameworkWindowWidth / float(_frameworkWindowHeight), 0.1f, 5000.f);
        camera->projection[1][1] *= -1;
    }
};

int main()
{
    Particles f;
    try {
        f.run(0, 0, false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    std::cout << "DONE\n";
}