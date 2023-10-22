# TGA
## Trainings Graphics API

### Development Environment

#### Dependencies
Building TGA is dependent on the following:
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) **Installation Required**
- [GLFW](https://www.glfw.org/) 
- [GLM](https://github.com/g-truc/glm) 
- [stb single-file public domain libraries ](https://github.com/nothings/stb)
- (optional, tga_utils only)[tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)

GLFW, GLM and stb are included as submodules and are optional to install globally.

The Vulkan SDK and Vulkan Validation Layers need to be installed manually.

You can link against the cmake targets `tga_vulkan` and optionally `tga_utils`.

### API Documentation

#### Shader
A Shader represents code to be executed on the GPU.

A handle to a Shader is created with a call to ```Interface::createShader(const ShaderInfo &shaderInfo)```
The ShaderInfo struct has the following parameters:
```
struct ShaderInfo{
  ShaderType type;    // Type of Shader. Valid Types are ShaderType::vertex and ShaderType::fragment
  uint8_t const *src; // Pointer to the shader code. Dependant on the underlying API, for Vulkan this would be SPIR-V 
  size_t srcSize;     // Size of the shader code in bytes
```
The handle to a Shader is valid until a call to ```Interface::free(Shader shader)``` or until the destruction of the interface


### StagingBuffer
A StagingBuffer represents a chunk of memory that can be used to transfer data to the GPU.

A handle to a StagingBuffer is created with a call to ```Interface::createStagingBuffer(const StagingBufferInfo &stagingBufferInfo)```
The StagingBufferInfo struct has the following parameters:
```
struct StagingBufferInto{
  size_t dataSize;     /**<Size of the buffer data in bytes*/
  uint8_t const *data; /**<Data of the Buffer to be uploaded. Alignment requirements are the users responsibility*/
```


A (void) pointer to the memory managed by the StagingBuffer can be obtained with a call to ```Interface::getMapping(StagingBuffer stagingBuffer)```

The handle to a StagingBuffer is valid until a call to ```Interface::free(StagingBuffer stagingBuffer)``` or until the destruction of the interface


#### Buffer
A Buffer represents a chunk of memory on the GPU.

A handle to a Buffer is created with a call to ```Interface::createBuffer(const BufferInfo &bufferInfo)```
The BufferInfo struct has the following parameters:
```
struct BufferInfo{
  BufferUsage usage; /**<Usage flags of the Buffer. Flags can be combined with the | operator*/
  size_t size;       /**<Size of the buffer data in bytes*/
  StagingBuffer srcData; /**<(optional) Data of the Buffer to be uploaded.*/
  size_t srcDataOffset;  /**<Offset from the start of the staging buffer*/
```
Note: To update or retrieve the contents of a buffer checkout the `bufferUpload` and `bufferDownload` commands 

The handle to a Buffer is valid until a call to ```Interface::free(Buffer buffer)``` or until the destruction of the interface

#### Texture
A Texture represents an image that is stored on and used by the GPU.

A handle to a Texture is created with a call to ```Interface::createTexture(const TextureInfo &textureInfo)```
The TextureInfo struct has the following parameters:
```
struct TextureInfo{
  uint32_t width;          /**<Width of the Texture in pixels*/
    uint32_t height;         /**<Height of the Texture in pixels*/
    Format format;           /**<Format of the pixels. Example: For 8 bit per Pixel with red, reen and blue channel use Format::r8g8b8_unorm. For a list of all formats refer to tga::Format*/
    SamplerMode samplerMode; /**<How the Texture is sampled. Valid SamplerModes are SamplerMode::nearest (default) and SamplerMode::linear*/
    AddressMode addressMode; /**<How textures reads with uv-coordinates outside of [0:1] are handled. For a list of all repeat modes refer to tga::AddressMode*/
    TextureType textureType; /**<Type of the texture, by default 2D*/
    uint32_t depthLayers;  /**<If texture type is not 2D, this describes the third dimension of the image. Must be 6 for Cube */
    StagingBuffer srcData; /**<(optional) Data of the Texture. Pass a TGA_NULL_HANDLE to create a texture with undefined content*/
    size_t srcDataOffset;  /**<Offset from the start of the staging buffer*/
```
Note: To retrieve the contents of a texture checkout the 'textureDownload' command 


The handle to a Texture is valid until a call to ```Interface::free(Texture texture)``` or until the destruction of the interface

#### Window
A Window is used to present the result of a fragment shader to the screen.

A handle to a Window is created with a call to ```Interface::createWindow(const WindowInfo &windowInfo)```
The WindowInfo struct has the following parameters:
```
struct WindowInfo{
  uint32_t width;             // Width of the Window in pixels
  uint32_t height;            // Height of the Window in pixels
  PresentMode presentMode;    // How syncronization to the monitor is handled. Valid PresentModes are PresentMode::immediate (show frame as fast as possible, default) and PresentMode::vsync (sync to the monitor refresh rate) 
  uint32_t framebufferCount;  // How many backbuffers the window has to manage. Due to minimum and maximum contraints this value may not be the actual resulting number of backbuffers and needs to be polled later
```
The handle to a Window can be used to query and update its state:
- To get the number of framebuffers used by a window call ```Interface::backbufferCount(Window window)```
- To get the index of the next backbuffer that can be written to and to poll input events call ```Interface::nextFrame(Window window)```
- To show a frame that has been acquired with `nextFrame` call ```Interface::present(Window window, uint32_t backbufferIndex)```
- To change the title of the Window call ```Interface::setWindowTitle(Window window, const std::string &title)```
- To manually poll new window events call ```Interface::pollEvents(Window window)```
- To find out if a user wished to close the Window call ```Interface::windowShouldClose(Window window)```
- To find out if a certain keyboard or mouse key was pressed during the last `nextFrame` call or the last `pollEvents` call, call ```Interface::keyDown(Window window, Key key)```
- To find the position of the mouse cursor relative to a Window in pixel coordinates call ```Interface::mousePosition(Window window)```

The handle to a Window is valid until a call to ```Interface::free(Window window)``` or until the destruction of the interface

#### InputSet
An InputSet is a collection of Bindings and a Binding is a resource used in a Shader.

A handle to an InputSet is created with a call to ```Interface::createInputSet(const InputSetInfo &inputSetInfo)```
The InputSetInfo struct has the following parameters:
```
struct InputSetInfo{
  using TargetPass = std::variant<RenderPass, ComputePass>;
  TargetPass targetPass;         /**<The RenderPass this InputSet should be used with*/
  std::vector<Binding> bindings; /**<The collection of Bindings in this InputSet*/
  uint32_t index;                /**<The Index of this InputSet as defined in RenderPass.inputLayout*/
```
##### Binding
A Binding assigns a resource to a shader as declared in RenderPass::InputLayout::SetLayout
The Binding struct consists of:
- ```std::variant<Buffer, Texture> resource``` The handle the resource that should be bound
- ```uint32_t slot```The index of the Binding in the shader
- ```uint32_t arrayElement```The index of the Binding into the array if specified, zero by default
The handle to an InputSet is valid until a call to ```Interface::free(InputSet inputSet)``` or until the destruction of the interface

#### RenderPass
A RenderPass describes a configuration of the graphics-pipeline.

A handle to a RenderPass is created with a call to ```Interface::createRenderPass(const RenderPassInfo &renderPassInfo)```
The RenderPassInfo struct has the following parameters:
```
struct RenderPassInfo{
  Shader vertexShader; /**<The vertex shader executed by this RenderPass*/
    Shader fragmentShader; /**<The fragment shader executed by this RenderPass*/
    
    using RenderTarget = std::variant<Texture, Window, std::vector<Texture>>;
    RenderTarget renderTarget{}; /**<Where the result of the fragment shader stage will be saved. Keep in mind that a Window can have several framebuffers*/
    VertexLayout vertexLayout{}; /**<Describes the format of the vertices in the vertex buffer*/
    InputLayout inputLayout{};   /**<Describes how the Bindings are organized*/

    ClearOperation clearOperations{ClearOperation::none}; /**<Determines if the renderTarget and/or depth-buffer should be cleared*/
    PerPixelOperations perPixelOperations{};              /**<Describes operations on each sample, i.e depth-buffer and blending*/
    RasterizerConfig rasterizerConfig{};                  /**<Describes the configuration the Rasterizer, i.e culling and polygon draw mode*/
```
##### VertexLayout
The VertexLayout describes how a vertex in a vertex-buffer is laid out in memory.
The VertexLayout struct consists of:
- ```size_t vertexSize``` The size of the complete vertex in bytes
- ```std::vector<VertexAttribute> vertexAttributes```The collection of attributes of the given vertex
A VertexAttribute consists of:
- ```size_t offset``` The offset of the attribute in bytes from the beginning of the vertex
- ```Format format``` The format of this attribute. i.e a vec4/float4 would be Format::r32g32b32a32_sfloat
	
##### RasterizerConfig
The RasterizerConfig determines culling and polygon-draw-mode
The RasterizerConfig struct consists of:
- ```FrontFace frontFace```Which face of a given triangle is the front face, either FrontFace::clockwise or FrontFace::counterClockwise
- ```CullMode cullMode```Which face to cull. Either CullMode::none, CullMode::front, CullMode::back or CullMode::all 
- ```PolygonMode polygonMode```Whether trinagles should be filled (PolygonMode::fill) or only drawn as wireframe (PolygonMode::wireframe)
##### PerPixelOperations
PerPixelOperations determines depth test and blending
The PerPixelOperations struct consists of:
- ```CompareOperation depthCompareOp``` If and how the depth test is performed. CompareOperation::ignore to disable depth-testing otherwise the equivalent of ==, <, <=, >, >=
- ```bool blendEnabled``` Whether blending is enabled
- ```BlendFactor srcBlend```The factor with which the source image rgb is weighted, by default BlendFactor::srcAlpha 
- ```BlendFactor dstBlend```The factor with which the destination image rgb is weighted, by default BlendFactor::oneMinusSrcAlpha
- ```BlendFactor srcAlphaBlend```The factor with which the source image alpha is weighted, by default BlendFactor::one 
- ```BlendFactor dstAlphaBlend```The factor with which the destination image alpha is weighted, by default BlendFactor::oneMinusSrcAlpha
##### InputLayout
The InputLayout describes how Bindings are organized.
The InputLayout is a collection of SetLayouts.
A SetLayout is a collection of BindingLayouts.
The BindingLayout struct consists of:
- ```BindingType type``` The type of Binding. Either BindingType::sampler for a (2D) texture, BindingType::uniformBuffer for a uniform-buffer or BindingType::storageBuffer for a storage-buffer
- ```uint32_t count``` The number of Bindings of the specified type. When count > 1 it is equivalent to an array of this BindingType in the shader programm 


The handle to a RenderPass is valid until a call to ```Interface::free(RenderPass renderPass)``` or until the destruction of the interface

### ComputePass
A ComputePass describes a configuration of the compute-pipeline.


A handle to a ComputePass is created with a call to ```Interface::createComputerPass(const ComputePassInfo &computePassInfo)```
The ComputePassInfo struct has the following parameters:
```
struct ComputePassInfo{
  Shader computeShader;    /**<The Shader to be executed in this ComoutePass.*/
  InputLayout inputLayout; /**<Describes how the Bindings are organized*/
```

The handle to a ComputePass is valid until a call to ```Interface::free(ComputePass computePass)``` or until the destruction of the interface

#### CommandBuffer
A CommandBuffer is a list of instructions to be executed by the GPU.

To start recording a CommandBuffer, create an instance of ```tga::CommandRecorder```.

A CommandRecorder can be initialized with a previously executed command buffer. In this case, it will wait for completion and then override the command buffer with new commands.

A handle to an already valid CommandBuffer can be passed to _beginCommandBuffer_ to clear it and begin recording of a new set of commands.
Is obtained upon calling ```CommandRecorder::endRecording()```. This function must be called only once. Afterwards the CommandRecorder should be discarded as no new commands are permitted to be recorded afterwards.


The following list of commands is available:
- ```setRenderPass(RenderPass renderPass, uint32_t framebufferIndex,std::array<float, 4> const& colorClearValue = {}, float depthClearValue = 1.0f)``` Configure the graphics pipeline to use the specified RenderPass targeting the specified framebuffer of RenderPass.renderTarget. If clear operations are set, corresponding clear values are applied.
- ```void setComputePass(ComputePass computePass)``` Configure the compute pipeline to use the specified compute shader
- ```bindVertexBuffer(Buffer buffer)```Use a Buffer as a vertex-buffer
- ```bindIndexBuffer(Buffer buffer)```Use a Buffer as an index-buffer
- ```bindInputSet(InputSet inputSet)```Bind all Bindings specified in the InputSet 
- ```draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount=1, uint32_t firstInstance=0)```Issue a draw command with the number of vertices and an offset into the currently bound vertex-buffer. Changing the values for instanceCount and firstInstance can be used for instanced rendering.
- ```drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t instanceCount=1, uint32_t firstInstance=0)```Issue am indexed draw command with the number of indices, an offset into the currently bound index-buffer and an offset into the currently bound vertex-buffer. Changing the values for instanceCount and firstInstance can be used for instanced rendering.
- ```drawIndirect(CommandBuffer, Buffer indirectDrawBuffer, uint32_t drawCount, size_t offset, uint32_t stride)``` Same as `draw` command but uses contents of _indirectDrawBuffer_ as arguments
- ```drawIndexedIndirect(CommandBuffer, Buffer indirectDrawBuffer, uint32_t drawCount, size_t offset, uint32_t stride)``` Same as `drawIndexed` command but uses contents of _indirectDrawBuffer_ as arguments
- ```dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)```Dispatches a compute shader with the specified number of work groups in each dimension. Each dimension must be greater than zero
- ```barrier(PipelineStage srcStage, PipelineStage dstStage)``` Places an execution and memory barrier between the source stage _srcStage_ and the destination stage _dstStage_. This is necessary to prevent race conditions between concurrently running commands
- ```inlineBufferUpdate(Buffer dst, void const *srcData, uint16_t dataSize, size_t dstOffset)``` Record a small amount of data into the command buffer to update the contents of a buffer. The _dataSize_ must be a multiple of 4.
- ```bufferUpload(StagingBuffer src, Buffer dst, size_t size, size_t srcOffset, size_t dstOffset)``` Transfer contents from the source StagingBuffer to the destination Buffer upon execution of the CommandBuffer. (CPU to GPU)
- ```bufferDownload(Buffer src, StagingBuffer dst, size_t size, size_t srcOffset, size_t dstOffset)``` Transfer contents from the source Buffer to the destination StagingBuffer upon execution of the CommandBuffer. (GPU to CPU)
- ```textureDownload(Texture src, StagingBuffer dst, size_t dstOffset)```Transfer contents from the source Texture to the destination StagingBuffer upon execution of the CommandBuffer. (GPU to CPU)

To execute a CommandBuffer call ```Interface::execute(CommandBuffer commandBuffer)```

To await the completion of a CommandBuffer call ```Interface::waitForCompletion(CommandBuffer commandBuffer)```

The handle to a CommandBuffer is valid until a call to ```Interface::free(CommandBuffer commandBuffer)``` or until the destruction of the interface



