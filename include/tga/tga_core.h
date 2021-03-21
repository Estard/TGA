#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>

// Heavily inspired by Vulkan
#define TGA_NULL_HANDLE 0
#define TGA_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define TGA_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T* object;

TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaShader)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaBuffer)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaTexture)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaWindow)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaInputSet)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaRenderPass)
TGA_DEFINE_NON_DISPATCHABLE_HANDLE(TgaCommandBuffer)

#ifdef __cplusplus
}
#endif