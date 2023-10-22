#include <chrono>

#include "tga/tga.hpp"
#include "tga/tga_utils.hpp"

struct BufferParams {
    float a;
    uint32_t size;
};

// Reference implementation
static void saxpy(BufferParams const& params, float *x, float *y, float *z)
{
    float a = params.a;
    auto start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < params.size; ++i) {
        z[i] = a * x[i] + y[i];
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "CPU Execution Time: " << std::chrono::duration<double,std::milli>(end - start).count() << "ms\n";
}

int main()
{
    tga::Interface tgai;
    auto saxpyShader = tga::loadShader("../shaders/saxpy_comp.spv", tga::ShaderType::compute, tgai);

    tga::InputLayout inputLayout{tga::SetLayout{tga::BindingType::uniformBuffer, tga::BindingType::storageBuffer,
                                                tga::BindingType::storageBuffer, tga::BindingType::storageBuffer}};

    auto computePass = tgai.createComputePass({saxpyShader, inputLayout});

    // My integrated GPU only has 2048 MB, this is
    BufferParams params{6.9, (1 << 27) + (1 << 20) + 17};

    auto bufferSize = params.size * sizeof(float);

    auto paramsStaging = tgai.createStagingBuffer({sizeof(params), tga::memoryAccess(params)});
    auto paramBuf = tgai.createBuffer({tga::BufferUsage::uniform, sizeof(params), paramsStaging});

    auto xStaging = tgai.createStagingBuffer({bufferSize});
    auto yStaging = tgai.createStagingBuffer({bufferSize});

    // Staging buffers can be written to after creation. This requires getting a pointer to memory
    auto x = static_cast<float *>(tgai.getMapping(xStaging));
    auto y = static_cast<float *>(tgai.getMapping(yStaging));

    for (uint32_t i = 0; i < params.size; ++i) {
        x[i] = static_cast<float>(i);
        y[i] = x[i] * 3.0 + 1.0;
    }
    auto xBuf = tgai.createBuffer({tga::BufferUsage::storage, bufferSize, xStaging});
    auto yBuf = tgai.createBuffer({tga::BufferUsage::storage, bufferSize, yStaging});
    auto zBuf = tgai.createBuffer({tga::BufferUsage::storage, bufferSize});

    auto inputSet = tgai.createInputSet(
        tga::InputSetInfo(computePass, {tga::Binding{paramBuf, 0}, {xBuf, 1}, {yBuf, 2}, {zBuf, 3}}, 0));

    auto resultSB = tgai.createStagingBuffer({bufferSize});

    constexpr auto workGroupSize = 64;
    auto cmd = tga::CommandRecorder(tgai)
                   .setComputePass(computePass)
                   .bindInputSet(inputSet)
                   .dispatch((params.size + (workGroupSize - 1)) / workGroupSize, 1, 1)
                   .endRecording();
    auto start = std::chrono::steady_clock::now();
    tgai.execute(cmd);
    tgai.waitForCompletion(cmd);
    auto end = std::chrono::steady_clock::now();

    auto getResultCmd =
        tga::CommandRecorder(tgai)
            // barrier here is not necessary since the results will be available after waiting for command completion
            // if the download should be recorded in the same commandbuffer, the barrier would be necessary
            .barrier(tga::PipelineStage::ComputeShader, tga::PipelineStage::Transfer)
            .bufferDownload(zBuf, resultSB, bufferSize)
            .endRecording();
    auto transferStart = std::chrono::steady_clock::now();
    tgai.execute(getResultCmd);
    tgai.waitForCompletion(getResultCmd);
    auto transferEnd = std::chrono::steady_clock::now();

    auto z = static_cast<float *>(tgai.getMapping(resultSB));
    for (uint32_t i = 0; i < params.size; ++i) {
        auto expected = params.a * x[i] + y[i];
        if (z[i] != expected) {
            std::cerr << "Index " << i << " is wrong, expected " << expected << " but got " << z[i] << " instead\n";
            break;
        }
    }

    std::cout << "Saxpy Compute: Success\n";
    std::cout << "Vector Size: " << params.size << '\n';
    std::cout << "GPU Execution time: " << std::chrono::duration<double, std::milli>(end - start).count() << "ms\n";
    std::cout << "Buffer Readback time: " << std::chrono::duration<double, std::milli>(transferEnd - transferStart).count() << "ms\n";
    
    saxpy(params, x, y, z);

    return 0;
}