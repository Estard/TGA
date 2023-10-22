#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

#include "tga_format.hpp"
#include "tga_resource_handles.hpp"

namespace tga
{

//
#define TGA_SETTER(FUNCNAME, TYPE, NAME) \
    auto& FUNCNAME(TYPE _##NAME)         \
    {                                    \
        NAME = _##NAME;                  \
        return *this;                    \
    }

/* Shader
 */

enum class ShaderType { vertex, fragment, compute };

struct ShaderInfo {
    ShaderType type;    /**<Type of Shader. Valid Types are ShaderType::vertex and
                              ShaderType::fragment*/
    uint8_t const *src; /**<Pointer to the shader code. Dependant on the
                          underlying API. For Vulkan this would be SPIR-V*/
    size_t srcSize;     /**<Size of the shader code in bytes*/
    ShaderInfo(ShaderType _type, uint8_t const *_src, size_t _srcSize) : type(_type), src(_src), srcSize(_srcSize) {}
    ShaderInfo(ShaderType _type, std::vector<uint8_t>& _src) : type(_type), src(_src.data()), srcSize(_src.size()) {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setType, ShaderType, type)
    TGA_SETTER(setSrc, uint8_t const *, src)
    TGA_SETTER(setSrcSize, size_t, srcSize)
};

/* Buffer
 */

/* Staging Buffer
 */
struct StagingBufferInfo {
    size_t dataSize;     /**<Size of the buffer data in bytes*/
    uint8_t const *data; /**<Data of the Buffer to be uploaded. Alignment requirements are the users responsibility*/
    StagingBufferInfo(size_t _dataSize, uint8_t const *_data = nullptr) : dataSize(_dataSize), data(_data) {}
    StagingBufferInfo(std::vector<uint8_t> const& _data = std::vector<uint8_t>())
        : dataSize(_data.size()), data(_data.data())
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setDataSize, size_t, dataSize)
    TGA_SETTER(setData, uint8_t const *, data)
};

enum class BufferUsage : uint32_t {
    undefined = 0u,
    uniform = 1u << 0u,
    vertex = 1u << 1u,
    index = 1u << 2u,
    storage = 1u << 3u,
    indirect = 1u << 4u,
    // ext
    accelerationStructureBuildInput = 1u << 5u
};
inline BufferUsage operator|(BufferUsage a, BufferUsage b)
{
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline bool operator&(BufferUsage a, BufferUsage b)
{
    return bool(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

struct BufferInfo {
    BufferUsage usage; /**<Usage flags of the Buffer. Flags can be combined with the | operator*/
    size_t size;       /**<Size of the buffer data in bytes*/
    StagingBuffer srcData; /**<(optional) Data of the Buffer to be uploaded. */
    size_t srcDataOffset;  /**<Offset from the start of the staging buffer*/

    BufferInfo(BufferUsage _usage, size_t _size, StagingBuffer _srcData = {}, size_t _srcDataOffset = 0)
        : usage(_usage), size(_size), srcData(_srcData), srcDataOffset(_srcDataOffset)
    {}

    TGA_SETTER(setUsage, BufferUsage, usage)
    TGA_SETTER(setSize, size_t, size)
    TGA_SETTER(setSrcData, StagingBuffer, srcData)
    TGA_SETTER(setSrcDataOffset, size_t, srcDataOffset)
};

/* Texture
 */

enum class SamplerMode { nearest, linear };

enum class AddressMode { clampBorder, clampEdge, repeat, repeatMirror };

enum class TextureType { _2D, _2DArray, _3D, _Cube };

struct TextureInfo {
    uint32_t width;          /**<Width of the Texture in pixels*/
    uint32_t height;         /**<Height of the Texture in pixels*/
    Format format;           /**<Format of the pixels. Example: For 8 bit per Pixel with red, green and blue channel use
                                Format::r8g8b8_unorm. For a list of all formats refer to tga::Format*/
    SamplerMode samplerMode; /**<How the Texture is sampled. Valid SamplerModes are SamplerMode::nearest (default) and
                                SamplerMode::linear*/
    AddressMode addressMode; /**<How textures reads with uv-coordinates outside of [0:1] are handled. For a list of all
                                repeat modes refer to tga::AddressMode*/
    TextureType textureType; /**<Type of the texture, by default 2D*/
    uint32_t depthLayers;  /**<If texture type is not 2D, this describes the third dimension of the image. Must be 6 for
                              Cube */
    StagingBuffer srcData; /**<(optional) Data of the Texture. Pass a TGA_NULL_HANDLE to create a texture with undefined content*/
    size_t srcDataOffset;  /**<Offset from the start of the staging buffer*/

    TextureInfo(uint32_t _width, uint32_t _height, Format _format, SamplerMode _samplerMode = SamplerMode::nearest,
                AddressMode _repeateMode = AddressMode::clampBorder, TextureType _textureType = TextureType::_2D,
                uint32_t _depthLayers = 1, StagingBuffer _srcData = {}, size_t _srcDataOffset = 0)
        : width(_width), height(_height), format(_format), samplerMode(_samplerMode), addressMode(_repeateMode),
          textureType(_textureType), depthLayers(_depthLayers), srcData(_srcData), srcDataOffset(_srcDataOffset)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setWidth, uint32_t, width)
    TGA_SETTER(setHeight, uint32_t, height)
    TGA_SETTER(setFormat, Format, format)
    TGA_SETTER(setSamplerMode, SamplerMode, samplerMode)
    TGA_SETTER(setAddressMode, AddressMode, addressMode)
    TGA_SETTER(setTextureType, TextureType, textureType)
    TGA_SETTER(setDepthLayers, uint32_t, depthLayers)
    TGA_SETTER(setSrcData, StagingBuffer, srcData)
    TGA_SETTER(setSrcDataOffset, size_t, srcDataOffset)
};

/* Window
 */

enum class PresentMode {
    immediate,
    vsync,
};

struct WindowInfo {
    uint32_t width;            /**<Width of the Window in pixels*/
    uint32_t height;           /**<Height of the Window in pixels*/
    PresentMode presentMode;   /**<How syncronization to the monitor is handled. Valid
                                    PresentModes are PresentMode::immediate (show frame as
                                    fast as possible, default) and PresentMode::vsync (sync to
                                    the monitor refresh rate)*/
    uint32_t framebufferCount; /**<How many backbuffers the window has to manage.
                                  Due to minimum and maximum contraints this value
                                  may not be the actual resulting number of
                                  backbuffers and needs to be polled later*/
    WindowInfo(uint32_t _width, uint32_t _height, PresentMode _presentMode = PresentMode::immediate,
               uint32_t _framebufferCount = 2)
        : width(_width), height(_height), presentMode(_presentMode), framebufferCount(_framebufferCount)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setWidth, uint32_t, width)
    TGA_SETTER(setHeight, uint32_t, height)
    TGA_SETTER(setPresentMode, PresentMode, presentMode)
    TGA_SETTER(setFramebufferCount, uint32_t, framebufferCount)
};

/* RenderPassInfo
 */

// Rasterizer Stage
enum class FrontFace { clockwise, counterclockwise };
enum class CullMode { none, front, back, all };
enum class PolygonMode { solid, wireframe };

struct RasterizerConfig {
    FrontFace frontFace;
    CullMode cullMode;
    PolygonMode polygonMode;
    RasterizerConfig(FrontFace _frontFace = FrontFace::clockwise, CullMode _cullMode = CullMode::none,
                     PolygonMode _polygonMode = PolygonMode::solid)
        : frontFace(_frontFace), cullMode(_cullMode), polygonMode(_polygonMode)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setFrontFace, FrontFace, frontFace)
    TGA_SETTER(setCullMode, CullMode, cullMode)
    TGA_SETTER(setPolygonMode, PolygonMode, polygonMode)
};

// Pixel Blending Stage
enum class CompareOperation { ignore, equal, greater, greaterEqual, less, lessEqual };
enum class BlendFactor { zero, one, srcAlpha, oneMinusSrcAlpha, dstAlpha, oneMinusDstAlpha };

struct PerPixelOperations {
    CompareOperation depthCompareOp;
    bool blendEnabled;
    BlendFactor srcBlend;
    BlendFactor dstBlend;
    BlendFactor srcAlphaBlend;
    BlendFactor dstAlphaBlend;
    PerPixelOperations(CompareOperation _depthCompareOp = CompareOperation::ignore, bool _blendEnabled = false,
                       BlendFactor _srcBlend = BlendFactor::srcAlpha,
                       BlendFactor _dstBlend = BlendFactor::oneMinusSrcAlpha,
                       BlendFactor _srcAlphaBlend = BlendFactor::one,
                       BlendFactor _dstAlphaBlend = BlendFactor::oneMinusSrcAlpha)
        : depthCompareOp(_depthCompareOp), blendEnabled(_blendEnabled), srcBlend(_srcBlend), dstBlend(_dstBlend),
          srcAlphaBlend(_srcAlphaBlend), dstAlphaBlend(_dstAlphaBlend)
    {}
    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setDepthCompareOp, CompareOperation, depthCompareOp)
    TGA_SETTER(setBlendEnabled, bool, blendEnabled)
    TGA_SETTER(setSrcBlend, BlendFactor, srcBlend)
    TGA_SETTER(setDstBlend, BlendFactor, dstBlend)
    TGA_SETTER(setSrcAlphaBlend, BlendFactor, srcAlphaBlend)
    TGA_SETTER(setDstAlphaBlend, BlendFactor, dstAlphaBlend)
};

// General Shader Input
enum class BindingType { uniformBuffer, sampler, storageBuffer, storageImage, accelerationStructure };

struct BindingLayout {
    BindingType type;
    uint32_t count;
    BindingLayout(BindingType _type, uint32_t _count = 1) : type(_type), count(_count) {}
    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setType, BindingType, type)
    TGA_SETTER(setCount, uint32_t, count)
};

using SetLayout = std::vector<BindingLayout>;
// struct SetLayout {
//     std::vector<BindingLayout> bindingLayouts;
//     SetLayout(const std::vector<BindingLayout>& _bindingLayouts = {}) : bindingLayouts(_bindingLayouts) {}
//     SetLayout(std::initializer_list<BindingLayout> _bindingLayouts) : bindingLayouts(_bindingLayouts){};
// };

using InputLayout = std::vector<SetLayout>;
// struct InputLayout {
//     std::vector<SetLayout> setLayouts;
//     InputLayout(const std::vector<SetLayout>& _setLayouts = {}) : setLayouts(_setLayouts) {}
// };

// Vertex Shader Input
struct VertexAttribute {
    size_t offset;
    Format format;
    VertexAttribute(size_t _offset = 0, Format _format = Format::undefined) : offset(_offset), format(_format) {}
    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setOffset, size_t, offset)
    TGA_SETTER(setFormat, Format, format)
};

struct VertexLayout {
    size_t vertexSize;
    std::vector<VertexAttribute> vertexAttributes;
    VertexLayout(size_t _vertexSize = 0,
                 std::vector<VertexAttribute> const& _vertexAttributes = std::vector<VertexAttribute>())
        : vertexSize(_vertexSize), vertexAttributes(_vertexAttributes)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setVertexSize, size_t, vertexSize)
    TGA_SETTER(setVertexAttributes, std::vector<VertexAttribute> const&, vertexAttributes)
};

enum class ClearOperation { none, color, depth, all };

struct RenderPassInfo {
    Shader vertexShader; /**<The vertex shader executed by this RenderPass*/
    Shader fragmentShader; /**<The fragment shader executed by this RenderPass*/
    
    using RenderTarget = std::variant<Texture, Window, std::vector<Texture>>;
    RenderTarget renderTarget{}; /**<Where the result of the fragment shader stage
                                  will be saved. Keep in mind that a Window can have several
                                  framebuffers*/
    VertexLayout vertexLayout{}; /**<Describes the format of the vertices in the vertex buffer*/
    InputLayout inputLayout{};   /**<Describes how the Bindings are organized*/

    ClearOperation clearOperations{ClearOperation::none}; /**<Determines if the renderTarget and/or
                                              depth-buffer should be cleared*/
    PerPixelOperations perPixelOperations{};              /**<Describes operations on each sample, i.e
                                                          depth-buffer and blending*/
    RasterizerConfig rasterizerConfig{};                  /**<Describes the configuration the Rasterizer, i.e
                                                               culling and polygon draw mode*/

    RenderPassInfo(Shader _vertexShader, Shader _fragmentShader, RenderTarget const& _renderTarget = {},
                   VertexLayout const& _vertexLayout = {}, InputLayout const& _inputLayout = {},
                   ClearOperation _clearOperations = ClearOperation::none,
                   PerPixelOperations const& _perPixelOperations = {}, RasterizerConfig const& _rasterizerConfig = {})
        : vertexShader(_vertexShader), fragmentShader(_fragmentShader), renderTarget(_renderTarget),
          vertexLayout(_vertexLayout), inputLayout(_inputLayout), clearOperations(_clearOperations),
          perPixelOperations(_perPixelOperations), rasterizerConfig(_rasterizerConfig)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setVertexShader, Shader, vertexShader)
    TGA_SETTER(setFragmentShader, Shader, fragmentShader)
    TGA_SETTER(setRenderTarget, Texture, renderTarget)
    TGA_SETTER(setRenderTarget, Window, renderTarget)
    TGA_SETTER(setRenderTarget, std::vector<Texture> const&, renderTarget)
    TGA_SETTER(setVertexLayout, VertexLayout const&, vertexLayout)
    TGA_SETTER(setInputLayout, InputLayout const&, inputLayout)
    TGA_SETTER(setClearOperations, ClearOperation, clearOperations)
    TGA_SETTER(setPerPixelOperations, PerPixelOperations, perPixelOperations)
    TGA_SETTER(setRasterizerConfig, RasterizerConfig, rasterizerConfig)
};

// ComputePass Info
struct ComputePassInfo {
    Shader computeShader;    /**<The Shader to be executed in this ComoutePass.*/
    InputLayout inputLayout; /**<Describes how the Bindings are organized*/

    // Constructor with single window
    ComputePassInfo(Shader const& _computeShader, InputLayout const& _inputLayout = InputLayout())
        : computeShader(_computeShader), inputLayout(_inputLayout)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setComputeShader, Shader, computeShader)
    TGA_SETTER(setInputLayout, InputLayout, inputLayout)
};

/* InputSet
 */

struct Binding {
    std::variant<Buffer, Texture> resource;
    uint32_t slot;
    uint32_t arrayElement;
    Binding(std::variant<Buffer, Texture> _resource, uint32_t _slot = 0, uint32_t _arrayElement = 0)
        : resource(_resource), slot(_slot), arrayElement(_arrayElement)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setResource, Buffer, resource)
    TGA_SETTER(setResource, Texture, resource)
    TGA_SETTER(setSlot, uint32_t, slot)
    TGA_SETTER(setArrayElement, uint32_t, arrayElement)
};

struct InputSetInfo {
    using TargetPass = std::variant<RenderPass, ComputePass>;
    TargetPass targetPass;         /**<The RenderPass this InputSet should be used with*/
    std::vector<Binding> bindings; /**<The collection of Bindings in this InputSet*/
    uint32_t index;                /**<The Index of this InputSet as defined in
                                                     RenderPass.inputLayout*/
    InputSetInfo(TargetPass _targetPass, std::vector<Binding> const& _bindings = {}, uint32_t _index = 0)
        : targetPass(_targetPass), bindings(_bindings), index(_index)
    {}

    // chaining setters for "Info().setX(x).setY(y)" pattern
    TGA_SETTER(setTargetPass, RenderPass, targetPass)
    TGA_SETTER(setTargetPass, ComputePass, targetPass)
    TGA_SETTER(setBindings, std::vector<Binding> const&, bindings)
    TGA_SETTER(setIndex, uint32_t, index)
};

/* CommandBuffer
 */

struct DrawIndirectCommand {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};
static_assert(sizeof(DrawIndirectCommand) == 4 * sizeof(uint32_t));

struct DrawIndexedIndirectCommand {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
};
static_assert(sizeof(DrawIndexedIndirectCommand) == 5 * sizeof(uint32_t));

/* Acceleration Structures
 */

namespace ext
{
    struct BottomLevelAccelerationStructureInfo {
        Buffer vertexBuffer;
        Buffer indexBuffer;
        size_t vertexStride;
        Format vertexPositionFormat;
        uint32_t maxVertex;
        uint32_t vertexCount;
        uint32_t firstVertex;
        uint32_t vertexOffset;
    };

    struct AccelerationStructureInstanceInfo {
        BottomLevelAccelerationStructure blas;
        std::array<std::array<float, 4>, 3> transform;
    };

    struct TopLevelAccelerationStructureInfo {
        std::vector<AccelerationStructureInstanceInfo> instanceInfos;
    };
}  // namespace ext

#undef TGA_CHAINSET
}  // namespace tga