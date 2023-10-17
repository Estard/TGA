#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>

// C style handles
#define TGA_NULL_HANDLE 0
#define TGA_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;

TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaShader)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaBuffer)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaStagingBuffer)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaTexture)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaWindow)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaInputSet)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaRenderPass)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaComputePass)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaCommandBuffer)

TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaBottomLevelAccelerationStructure)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaTopLevelAccelerationStructure)

#undef TGA_DEFINE_NON_DISPATCHABLE_HANDLE

#ifdef __cplusplus
}
#endif

#include <cstddef>

#define TGA_TYPE_SAFE_HANDLE_STRUCT(NAME)              \
    struct NAME {                                      \
        NAME() : handle(TGA_NULL_HANDLE)               \
        {}                                             \
        NAME(std::nullptr_t) : handle(TGA_NULL_HANDLE) \
        {}                                             \
        NAME(Tga##NAME _handle) : handle(_handle)      \
        {}                                             \
        NAME &operator=(Tga##NAME _handle)             \
        {                                              \
            this->handle = _handle;                    \
            return *this;                              \
        }                                              \
        operator Tga##NAME() const                     \
        {                                              \
            return handle;                             \
        }                                              \
        explicit operator bool() const                 \
        {                                              \
            return handle != TGA_NULL_HANDLE;          \
        }                                              \
        bool operator!() const                         \
        {                                              \
            return handle == TGA_NULL_HANDLE;          \
        }                                              \
                                                       \
    private:                                           \
        Tga##NAME handle;                              \
    }

namespace tga
{
    /** \brief A Shader represents code to be executed on the GPU.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(Shader);

    /** \brief A Buffer represents a chunk of memory on the GPU.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(Buffer);

    /** \brief A StagingBuffer represents a chunk of memory on the CPU that is readable by the GPU.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(StagingBuffer);

    /** \brief A Texture represents an image that is stored on and used by the GPU.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(Texture);

    /** \brief A Window is used to present the result of a fragment shader to the screen.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(Window);

    /** \brief An InputSet is a collection of Bindings and a Binding is a resource used in a Shader.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(InputSet);

    /** \brief A RenderPass describes a configuration of the graphics-pipeline.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(RenderPass);

     /** \brief A ComputePass describes a configuration of the compute-pipeline.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(ComputePass);

    /** \brief A CommandBuffer is a list of instructions to be executed by the GPU.
     */
    TGA_TYPE_SAFE_HANDLE_STRUCT(CommandBuffer);

    namespace ext
    {
        TGA_TYPE_SAFE_HANDLE_STRUCT(BottomLevelAccelerationStructure);
        TGA_TYPE_SAFE_HANDLE_STRUCT(TopLevelAccelerationStructure);

    }  // namespace ext

}  // namespace tga
#undef TGA_TYPE_SAFE_HANDLE_STRUCT