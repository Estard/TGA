# TGA
## Trainings Graphics API

### Development Environment

#### Dependencies
Building TGA is dependent on the following:
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) **Installation Required**
- [GLFW](https://www.glfw.org/) 
- [GLM](https://github.com/g-truc/glm) 
- [stb single-file public domain libraries ](https://github.com/nothings/stb)

GLFW, GLM and stb are included as submodules and are optional to install globally

Should, after installing the vulkan SDK, simply loading the CMake project not work, try to set up the development environment manually
- For the dependencies follow the instructions from [vulkan-tutorial.com](https://vulkan-tutorial.com/Development_environment)
- Add TGA/include to additional includes
- Add the source file of TGA/src/tga_*api_you_want_use* to your project
- Add the source file of TGA/src/tga_*api_you_want_use*/WSI_*window_library* to your project
- If on Windows: Add NOMINMAX to preprocessor defines

Currently implemented are:
- Vulkan API using GLFW as window library

### API Documentation

#### Shader
A Shader represents code to be executed on the GPU.

A handle to a Shader is created with a call to ```Interface::createShader(const ShaderInfo &shaderInfo);```
The ShaderInfo struct requires the following parameters:
```
struct ShaderInfo{
  ShaderType type;    // Type of Shader. Valid Types are ShaderType::vertex and ShaderType::fragment
  uint8_t const *src; // Pointer to the shader code. Dependant on the underlying API, for Vulkan this would be SPIR-V 
  size_t srcSize;     // Size of the shader code in bytes
```
The handle to a Shader is valid until a call to ```Interface::free(Shader shader);``` or until the destruction of the interface

#### Buffer
A Buffer represents a chunk of memory on the GPU.

A handle to a Buffer is created with a call to ```Interface::createBuffer(const BufferInfo &bufferInfo);```
The BufferInfo struct requires the following parameters:
```
struct BufferInfo{
  BufferUsage usage;    // Usage flags of the Buffer. Valid Usage flags are BufferUsage::uniform, BufferUsage::vertex and BufferUsage::index. Others are work in progress
  uint8_t const *data;  // Data of the Buffer to be uploaded. Alignment requirements are the users responsibility 
  size_t dataSize;      // Size of the buffer data in bytes
```
To update the contents of a Buffer call ```Interface::updateBuffer(Buffer buffer, uint8_t const *data, size_t dataSize, uint32_t offset)``` with the Buffer you want to update, the data you want to write, the size of the data in bytes and an offset from the beginning of the Buffer

The handle to a Buffer is valid until a call to ```Interface::free(Buffer buffer);``` or until the destruction of the interface

#### Texture
A Texture represents an image that is stored on and used by the GPU.

A handle to a Texture is created with a call to ```Interface::createTexture(const TextureInfo &textureInfo);```
The TextureInfo struct requires the following parameters:
```
struct TextureInfo{
  uint32_t width;           // Width of the Texture in pixels
  uint32_t height;          // Height of the Texture in pixels
  Format format;            // Format of the pixels. Example: For 8 bit per Pixel with red, green and blue channel use Format::r8g8b8_unorm. For a list of all formats refer to tga::Format
  uint8_t const *data;      // Data of the Texture. Pass a nullptr to create a texture with undefined content
  size_t dataSize;          // Size of the texture data in bytes
  SamplerMode samplerMode;  // How the Texture is sampled. Valid SamplerModes are SamplerMode::nearest (default) and   SamplerMode::linear
  AddressMode addressMode;    // How textures reads with uv-coordinates outside of [0:1] are handled. For a list of all addressing modes refer to tga::AddressMode
  TextureType textureType; // Type of the texture, by default 2D
  uint32_t depthLayers; // If texture type is not 2D, this describes the third dimension of the image. Must be 6 for Cube
```
The handle to a Texture is valid until a call to ```Interface::free(Texture texture);``` or until the destruction of the interface

#### Window
A Window is used to present the result of a fragment shader to the screen.

A handle to a Window is created with a call to ```Interface::createWindow(const WindowInfo &windowInfo);```
The WindowInfo struct requires the following parameters:
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
- To show the last frame that has been acquired with _nextFrame_ call ```Interface::present(Window window)```
- To change the title of the Window call ```Interface::setWindowTitle(Window window, const std::string &title)```
- To find out if a user wished to close the Window call ```Interface::windowShouldClose(Window window)```
- To find out if a certain keyboard or mouse key was pressed during _nextFrame_ call ```Interface::keyDown(Window window, Key key)```
- To find the position of the mouse cursor relative to a Window in pixel coordinates call ```Interface::mousePosition(Window window)```

The handle to a Window is valid until a call to ```Interface::free(Window window);``` or until the destruction of the interface

#### InputSet
An InputSet is a collection of Bindings and a Binding is a resource used in a Shader.

A handle to an InputSet is created with a call to ```Interface::createInputSet(const InputSetInfo &inputSetInfo);```
The InputSetInfo struct requires the following parameters:
```
struct InputSetInfo{
  RenderPass targetRenderPass;    // The RenderPass this InputSet should be used with
  uint32_t setIndex;              // The Index of this InputSet as defined in RenderPass.inputLayout  
  std::vector<Binding> bindings;  // The collection of Bindings in this InputSet
```
##### Binding
A Binding assigns a resource to a shader as declared in RenderPass::InputLayout::SetLayout
The Binding struct consists of:
- ```std::variant<Buffer, Texture> resource``` The handle the resource that should be bound
- ```uint32_t slot```The index of the Binding in the shader
- ```uint32_t arrayElement```The index of the Binding into the array if specified, zero by default
The handle to an InputSet is valid until a call to ```Interface::free(InputSet inputSet);``` or until the destruction of the interface

#### RenderPass
A RenderPass describes a configuration of the graphics-pipeline.

A handle to a RenderPass is created with a call to ```Interface::createRenderPass(const RenderPassInfo &renderPassInfo);```
The RenderPassInfo struct requires the following parameters:
```
struct RenderPassInfo{
  std::vector<Shader> shaderStages;           // The Shaders to be executed in this RenderPass. Must be ordererd in accordance with the shader stages of the graphics pipeline (i.e vertex before fragment, no duplicate stages, etc.). If using a compute shader it has to be the only stader stage
  std::variant<Texture, Window> renderTarget; // Where the result of the fragment shader stage will be saved. Keep in mind that a Window can have several framebuffers and only one is written at a time 
  ClearOperation clearOperations;             // Determines if the renderTarget and/or depth-buffer should be cleared
  RasterizerConfig rasterizerConfig;          // Describes the configuration the Rasterizer, i.e culling and polygon draw mode
  PerPixelOperations perPixelOperations;    // Describes operations on each pixel, i.e depth-buffer and blending*/./
  InputLayout inputLayout;                    // Describes how the Bindings are organized
  VertexLayout vertexLayout;                  // Describes the format of the vertices in the vertex-buffer
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


The handle to a RenderPass is valid until a call to ```Interface::free(RenderPass renderPass);``` or until the destruction of the interface

#### CommandBuffer
A CommandBuffer is a list of instructions to be executed by the GPU.

A CommandBuffer is started with a call to ```Interface::beginCommandBuffer()```
A handle to an already valid CommandBuffer can be passed to _beginCommandBuffer_ to clear it and begin recording of a new set of commands.
The handle to a CommandBuffer can then be created with a call to ```Interface::endCommandBuffer()```
Inbetween _beginCommandBuffer_ and _endCommandBuffer_ you can call the following commands to be recorded in the CommandBuffer:
- ```setRenderPass(RenderPass renderPass, uint32_t framebufferIndex)``` Configure the Pipeline to use the specified RenderPass and target the specified framebuffer of RenderPass.renderTarget
- ```bindVertexBuffer(Buffer buffer)```Use a Buffer as a vertex-buffer
- ```bindIndexBuffer(Buffer buffer)```Use a Buffer as an index-buffer
- ```bindInputSet(InputSet inputSet)```Bind all Bindings specified in the InputSet 
- ```draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount=1, uint32_t firstInstance=0)```Issue a draw command with the number of vertices and an offset into the currently bound vertex-buffer. Changing the values for instanceCount and firstInstance can be used for instanced rendering.
- ```drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t instanceCount=1, uint32_t firstInstance=0)```Issue am indexed draw command with the number of indices, an offset into the currently bound index-buffer and an offset into the currently bound vertex-buffer. Changing the values for instanceCount and firstInstance can be used for instanced rendering.
- ```dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)```Dispatches a compute shader with the specified number of work groups in each each dimension. Each dimension cannot be zero

To execute a CommandBuffer call ```Interface::execute(CommandBuffer commandBuffer)```

The handle to a CommandBuffer is valid until a call to ```Interface::free(CommandBuffer commandBuffer);``` or until the destruction of the interface



