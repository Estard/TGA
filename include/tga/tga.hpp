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

namespace tga
{

    /** \brief The abstract Interface to the Trainings Graphics API
     *
     */
    class Interface {
    public:
        // Resource Creation
        virtual ~Interface() = default;
        virtual Shader createShader(ShaderInfo const& shaderInfo) = 0;
        virtual Buffer createBuffer(BufferInfo const& bufferInfo) = 0;
        virtual Texture createTexture(TextureInfo const& textureInfo) = 0;
        virtual Window createWindow(WindowInfo const& windowInfo) = 0;
        virtual InputSet createInputSet(InputSetInfo const& inputSetInfo) = 0;
        virtual RenderPass createRenderPass(RenderPassInfo const& renderPassInfo) = 0;

        virtual ext::TopLevelAccelerationStructure createTopLevelAccelerationStructure(
            ext::TopLevelAccelerationStructureInfo const& TLASInfo) = 0;
        virtual ext::BottomLevelAccelerationStructure createBottomLevelAccelerationStructure(
            ext::BottomLevelAccelerationStructureInfo const& BLASInfo) = 0;

        // Commands
        virtual void beginCommandBuffer() = 0;
        virtual void beginCommandBuffer(CommandBuffer cmdBuffer) = 0;
        virtual void setRenderPass(RenderPass renderPass, uint32_t framebufferIndex) = 0;
        virtual void bindVertexBuffer(Buffer buffer) = 0;
        virtual void bindIndexBuffer(Buffer buffer) = 0;
        virtual void bindInputSet(InputSet inputSet) = 0;
        virtual void draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount = 1,
                          uint32_t firstInstance = 0) = 0;
        virtual void drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset,
                                 uint32_t instanceCount = 1, uint32_t firstInstance = 0) = 0;
        virtual void drawIndirect(Buffer buffer, uint32_t drawCount, size_t offset = 0,
                                  uint32_t stride = sizeof(tga::DrawIndirectCommand)) = 0;
        virtual void drawIndexedIndirect(Buffer buffer, uint32_t drawCount, size_t offset = 0,
                                         uint32_t stride = sizeof(tga::DrawIndexedIndirectCommand)) = 0;
        virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual CommandBuffer endCommandBuffer() = 0;
        virtual void execute(CommandBuffer commandBuffer) = 0;

        virtual void updateBuffer(Buffer buffer, uint8_t const* data, size_t dataSize, uint32_t offset) = 0;
        virtual std::vector<uint8_t> readback(Buffer buffer) = 0;
        virtual std::vector<uint8_t> readback(Texture texture) = 0;

        // Window functions

        /** \brief Number of framebuffers used by a window.
         * \return Number of framebuffers used as backbuffers by a window
         */
        virtual uint32_t backbufferCount(Window window) = 0;

        /** \brief Index of the next available framebuffer & polling of events.
         * \return Index of next available framebuffer
         */
        virtual uint32_t nextFrame(Window window) = 0;

        /** \brief Polling of events.
         */
        virtual void pollEvents(Window window) = 0;

        /** \brief Shows the last acquired framebuffer on screen.
         */
        virtual void present(Window window) = 0;

        /** \brief Changes title of window.
         * \param title New title of window
         */
        virtual void setWindowTitle(Window window, std::string const& title) = 0;

        /** \brief True if user has issued a close command (pressed x) on the window.
         */
        virtual bool windowShouldClose(Window window) = 0;

        /** \brief True if a key from keyboard or mouse was pressed during the last event poll.
         * \param key Key code of the mouse or keyboard key
         */
        virtual bool keyDown(Window window, Key key) = 0;

        /** \brief x and y position of mouse in pixel coordinates relative to window
         * \return pair (x,y) in pixel coordinates
         */
        virtual std::pair<int, int> mousePosition(Window window) = 0;

        /** \brief Resolution of the primary monitor in pixels
         * \return pair (width,height) in pixel
         */
        virtual std::pair<uint32_t, uint32_t> screenResolution() = 0;

        // Freedom
        virtual void free(Shader shader) = 0;
        virtual void free(Buffer buffer) = 0;
        virtual void free(Texture texture) = 0;
        virtual void free(Window window) = 0;
        virtual void free(InputSet inputSet) = 0;
        virtual void free(RenderPass renderPass) = 0;
        virtual void free(CommandBuffer commandBuffer) = 0;
    };
}  // namespace tga
