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
        Interface();
        ~Interface();
        // Resource Creation
        Shader createShader(ShaderInfo const& shaderInfo);
        Buffer createBuffer(BufferInfo const& bufferInfo);
        Texture createTexture(TextureInfo const& textureInfo);
        Window createWindow(WindowInfo const& windowInfo);
        InputSet createInputSet(InputSetInfo const& inputSetInfo);
        RenderPass createRenderPass(RenderPassInfo const& renderPassInfo);

        ext::TopLevelAccelerationStructure createTopLevelAccelerationStructure(
            ext::TopLevelAccelerationStructureInfo const& TLASInfo);
        ext::BottomLevelAccelerationStructure createBottomLevelAccelerationStructure(
            ext::BottomLevelAccelerationStructureInfo const& BLASInfo);

        // Commands
        void beginCommandBuffer();
        void beginCommandBuffer(CommandBuffer cmdBuffer);
        void setRenderPass(RenderPass renderPass, uint32_t framebufferIndex);
        void bindVertexBuffer(Buffer buffer);
        void bindIndexBuffer(Buffer buffer);
        void bindInputSet(InputSet inputSet);
        void draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount = 1,
                          uint32_t firstInstance = 0);
        void drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset,
                                 uint32_t instanceCount = 1, uint32_t firstInstance = 0);
        void drawIndirect(Buffer buffer, uint32_t drawCount, size_t offset,
                                  uint32_t stride = sizeof(tga::DrawIndirectCommand));
        void drawIndexedIndirect(Buffer buffer, uint32_t drawCount, size_t offset,
                                         uint32_t stride = sizeof(tga::DrawIndexedIndirectCommand));
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
        CommandBuffer endCommandBuffer();
        void execute(CommandBuffer commandBuffer);

        void updateBuffer(Buffer buffer, uint8_t const* data, size_t dataSize, uint32_t offset);
        std::vector<uint8_t> readback(Buffer buffer);
        std::vector<uint8_t> readback(Texture texture);

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

        /** \brief Shows the last acquired framebuffer on screen.
         */
        void present(Window window);

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
        void free(Shader shader);
        void free(Buffer buffer);
        void free(Texture texture);
        void free(Window window);
        void free(InputSet inputSet);
        void free(RenderPass renderPass);
        void free(CommandBuffer commandBuffer);

        private:
        struct InternalState;
        std::unique_ptr<InternalState> state;
    };
}  // namespace tga
