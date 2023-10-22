#pragma once
namespace tga
{

enum class PipelineStage {
    TopOfPipe = 0x1,
    DrawIndirect = 0x2,
    VertexInput = 0x4,
    VertexShader = 0x8,
    FragmentShader = 0x80,
    EarlyFragmentTests = 0x100,
    LateFragmentTests = 0x200,
    ColorAttachmentOutput = 0x400,
    ComputeShader = 0x800,
    Transfer = 0x1000,
    BottomOfPipe = 0x2000
};
}