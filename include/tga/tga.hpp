#pragma once
#include <algorithm>
#include <any>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "tga_createInfo_structs.hpp"
#include "tga_key_codes.hpp"
#include "tga_pipelinestages.hpp"

namespace tga
{

/** \brief The abstract Interface to the Trainings Graphics API
 *
 */
class Interface {
public:
    Interface();
    ~Interface();
    // Resource Creation
    Shader createShader(ShaderInfo const&);
    StagingBuffer createStagingBuffer(StagingBufferInfo const&);
    Buffer createBuffer(BufferInfo const&);
    Texture createTexture(TextureInfo const&);
    Window createWindow(WindowInfo const&);
    InputSet createInputSet(InputSetInfo const&);
    RenderPass createRenderPass(RenderPassInfo const&);
    ComputePass createComputePass(ComputePassInfo const&);

    ext::TopLevelAccelerationStructure createTopLevelAccelerationStructure(
        ext::TopLevelAccelerationStructureInfo const&);
    ext::BottomLevelAccelerationStructure createBottomLevelAccelerationStructure(
        ext::BottomLevelAccelerationStructureInfo const&);

    void execute(CommandBuffer);
    void waitForCompletion(CommandBuffer);

    void *getMapping(StagingBuffer);

    // Window functions

    /** \brief Number of framebuffers used by a window.
     * \return Number of framebuffers used as backbuffers by a window
     */
    uint32_t backbufferCount(Window window);

    /** \brief Index of the next available framebuffer & polling of events.
     * \return Index of next available framebuffer
     */
    uint32_t nextFrame(Window window);

    /** \brief Polling of events.
     */
    void pollEvents(Window window);

    /** \brief Shows the acquired framebuffer on screen.
     */
    void present(Window window, uint32_t framebufferIndex);

    /** \brief Changes title of window.
     * \param title New title of window
     */
    void setWindowTitle(Window window, std::string const& title);

    /** \brief True if user has issued a close command (pressed x) on the window.
     */
    bool windowShouldClose(Window window);

    /** \brief True if a key from keyboard or mouse was pressed during the last event poll.
     * \param key Key code of the mouse or keyboard key
     */
    bool keyDown(Window window, Key key);

    /** \brief x and y position of mouse in pixel coordinates relative to window
     * \return pair (x,y) in pixel coordinates
     */
    std::pair<int, int> mousePosition(Window window);

    /** \brief Resolution of the primary monitor in pixels
     * \return pair (width,height) in pixel
     */
    std::pair<uint32_t, uint32_t> screenResolution();

    // Freedom
    void free(Shader);
    void free(StagingBuffer);
    void free(Buffer);
    void free(Texture);
    void free(Window);
    void free(InputSet);
    void free(RenderPass);
    void free(ComputePass);
    void free(CommandBuffer);
    void free(ext::TopLevelAccelerationStructure);
    void free(ext::BottomLevelAccelerationStructure);

private:
    friend class CommandRecorder;
    CommandBuffer beginCommandBuffer(CommandBuffer cmdBuffer);
    void setRenderPass(CommandBuffer, RenderPass, uint32_t framebufferIndex,
                       std::array<float, 4> const& colorClearValue, float depthClearValue);
    void setComputePass(CommandBuffer, ComputePass);
    void bindVertexBuffer(CommandBuffer, Buffer);
    void bindIndexBuffer(CommandBuffer, Buffer);
    void bindInputSet(CommandBuffer, InputSet);
    void draw(CommandBuffer, uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount,
              uint32_t firstInstance);
    void drawIndexed(CommandBuffer, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset,
                     uint32_t instanceCount, uint32_t firstInstance);
    void drawIndirect(CommandBuffer, Buffer indirectDrawBuffer, uint32_t drawCount, size_t offset, uint32_t stride);
    void drawIndexedIndirect(CommandBuffer, Buffer indirectDrawBuffer, uint32_t drawCount, size_t offset,
                             uint32_t stride);
    void barrier(CommandBuffer, PipelineStage srcStage, PipelineStage dstStage);
    void dispatch(CommandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    void inlineBufferUpdate(CommandBuffer, Buffer dst, void const *srcData, uint16_t dataSize, size_t dstOffset);
    void bufferUpload(CommandBuffer, StagingBuffer src, Buffer dst, size_t size, size_t srcOffset, size_t dstOffset);
    void bufferDownload(CommandBuffer, Buffer src, StagingBuffer dst, size_t size, size_t srcOffset, size_t dstOffset);
    void textureDownload(CommandBuffer, Texture src, StagingBuffer dst, size_t dstOffset);
    void endCommandBuffer(CommandBuffer);

private:
    struct InternalState;
    std::unique_ptr<InternalState> state;
};

class CommandRecorder {
public:
    CommandRecorder(Interface& _tgai, CommandBuffer _cmdBuffer = {})
        : tgai(_tgai), cmdBuffer(tgai.beginCommandBuffer(_cmdBuffer))
    {}
    ~CommandRecorder()
    {
        if (cmdBuffer) tgai.endCommandBuffer(cmdBuffer);
    }

    CommandRecorder& setRenderPass(RenderPass renderPass, uint32_t framebufferIndex,
                                   std::array<float, 4> const& colorClearValue = {}, float depthClearValue = 1.0f)
    {
        tgai.setRenderPass(cmdBuffer, renderPass, framebufferIndex, colorClearValue, depthClearValue);
        return *this;
    }
    CommandRecorder& bindVertexBuffer(Buffer buffer)
    {
        tgai.bindVertexBuffer(cmdBuffer, buffer);
        return *this;
    }
    CommandRecorder& bindIndexBuffer(Buffer buffer)
    {
        tgai.bindIndexBuffer(cmdBuffer, buffer);
        return *this;
    }
    CommandRecorder& bindInputSet(InputSet inputSet)
    {
        tgai.bindInputSet(cmdBuffer, inputSet);
        return *this;
    }
    CommandRecorder& draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount = 1,
                          uint32_t firstInstance = 0)
    {
        tgai.draw(cmdBuffer, vertexCount, firstVertex, instanceCount, firstInstance);
        return *this;
    }
    CommandRecorder& drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset,
                                 uint32_t instanceCount = 1, uint32_t firstInstance = 0)
    {
        tgai.drawIndexed(cmdBuffer, indexCount, firstIndex, vertexOffset, instanceCount, firstInstance);
        return *this;
    }
    CommandRecorder& drawIndirect(Buffer buffer, uint32_t drawCount, size_t offset = 0,
                                  uint32_t stride = sizeof(tga::DrawIndirectCommand))
    {
        tgai.drawIndirect(cmdBuffer, buffer, drawCount, offset, stride);
        return *this;
    }
    CommandRecorder& drawIndexedIndirect(Buffer buffer, uint32_t drawCount, size_t offset = 0,
                                         uint32_t stride = sizeof(tga::DrawIndexedIndirectCommand))
    {
        tgai.drawIndexedIndirect(cmdBuffer, buffer, drawCount, offset, stride);
        return *this;
    }

    CommandRecorder& setComputePass(ComputePass computePass)
    {
        tgai.setComputePass(cmdBuffer, computePass);
        return *this;
    }

    CommandRecorder& barrier(PipelineStage srcStage, PipelineStage dstStage)
    {
        tgai.barrier(cmdBuffer, srcStage, dstStage);
        return *this;
    }

    CommandRecorder& dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        tgai.dispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);
        return *this;
    }

    CommandRecorder& inlineBufferUpdate(Buffer dst, void const *srcData, uint16_t dataSize, size_t dstOffset = 0)
    {
        tgai.inlineBufferUpdate(cmdBuffer, dst, srcData, dataSize, dstOffset);
        return *this;
    }
    CommandRecorder& bufferUpload(StagingBuffer src, Buffer dst, size_t size, size_t srcOffset = 0,
                                  size_t dstOffset = 0)
    {
        tgai.bufferUpload(cmdBuffer, src, dst, size, srcOffset, dstOffset);
        return *this;
    }
    CommandRecorder& bufferDownload(Buffer src, StagingBuffer dst, size_t size, size_t srcOffset = 0,
                                    size_t dstOffset = 0)
    {
        tgai.bufferDownload(cmdBuffer, src, dst, size, srcOffset, dstOffset);
        return *this;
    }

    CommandRecorder& textureDownload(Texture src, StagingBuffer dst, size_t dstOffset = 0)
    {
        tgai.textureDownload(cmdBuffer, src, dst, dstOffset);
        return *this;
    }

    CommandBuffer endRecording()
    {
        tgai.endCommandBuffer(cmdBuffer);
        auto result = cmdBuffer;
        cmdBuffer = {};
        return result;
    }

private:
    Interface& tgai;
    CommandBuffer cmdBuffer;
};

}  // namespace tga
