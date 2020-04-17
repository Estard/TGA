# TGA
## Trainings Graphics API

### Development Environment

Should simply loading the cmake project not work:
- For the dependencies follow the instructions from [vulkan-tutorial.com](https://vulkan-tutorial.com/Development_environment)
- Add TGA/include to additional includes
- Add the source file of TGA/src/tga_*api_you_want_use* to your project
- Add the source file of TGA/src/tga_*api_you_want_use*/WSI_*window_library* to your project

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
The handle to a Shader is valid until a call to ```Interface::free(Shader shader);``` or until the descruction of the interface

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

The handle to a Buffer is valid until a call to ```Interface::free(Buffer buffer);``` or until the descruction of the interface

#### Texture
A Texture represents an image that is stored on and used by the GPU.

A handle to a Texture is created with a call to ```Interface::createTexture(const TextureInfo &textureInfo);```
The TextureInfo struct requires the following parameters:
```
struct TextureInfo{
  uint32_t width;           // Width of the Texture in pixels
  uint32_t height;          // Height of the Texture in pixels
  uint8_t const *data;      // Data of the Texture. Pass a nullptr to create a texture with undefined content
  size_t dataSize;          // Size of the texture data in bytes
  Format format;            // Format of the pixels. Example: For 8 bit per Pixel with red, green and blue channel use Format::r8g8b8_unorm. For a list of all formats refer to tga::Format
  SamplerMode samplerMode;  // How the Texture is sampled. Valid SamplerModes are SamplerMode::nearest (default) and   SamplerMode::linear
  RepeatMode repeatMode;    // How textures reads with uv-coordinates outside of [0:1] are handled. For a list of all repeate modes refer to tga::RepeatMode
```
The handle to a Texture is valid until a call to ```Interface::free(Texture texture);``` or until the descruction of the interface

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

The handle to a Window is valid until a call to ```Interface::free(Window window);``` or until the descruction of the interface

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
The handle to an InputSet is valid until a call to ```Interface::free(InputSet inputSet);``` or until the descruction of the interface

#### RenderPass
A RenderPass describes a configuration of the graphics-pipeline.

A handle to a RenderPass is created with a call to ```Interface::createRenderPass(const RenderPassInfo &renderPassInfo);```
The RenderPassInfo struct requires the following parameters:
```
struct RenderPassInfo{
  std::vector<Shader> shaderStages;           // The Shaders to be executed in this RenderPass. Must be ordererd in accordance with the shader stages of the graphics pipeline (i.e vertex before fragment, no duplicate stages, etc.)
  std::variant<Texture, Window> renderTarget; // Where the result of the fragment shader stage will be saved. Keep in mind that a Window can have several framebuffers and only one is written at a time 
  ClearOperation clearOperations;             // Determines if the renderTarget and/or depth-buffer should be cleared
  VertexLayout vertexLayout;                  // Describes the format of the vertices in the vertex-buffer
  RasterizerConfig rasterizerConfig;          // Describes the configuration the Rasterizer, i.e blending, depth-buffer, culling and polygon draw mode
  InputLayout inputLayout;                    // Describes how the Bindings are organized
```
The handle to a RenderPass is valid until a call to ```Interface::free(RenderPass renderPass);``` or until the descruction of the interface

#### CommandBuffer
A CommandBuffer is a list of instructions to be executed by the GPU.

A CommandBuffer is started with a call to ```Interface::beginCommandBuffer(const CommandBufferInfo &commandBufferInfo)```
_Note that the CommandBufferInfo struct is currently empty_
The handle to a CommandBuffer can then be created with a call to ```Interface::endCommandBuffer()```
Inbetween _beginCommandBuffer_ and _endCommandBuffer_ you can call the following commands to be recorded in the CommandBuffer:
- ```setRenderPass(RenderPass renderPass, uint32_t framebufferIndex)``` Configure the Pipeline to use the specified RenderPass and target the specified framebuffer of RenderPass.renderTarget
- ```bindVertexBuffer(Buffer buffer)```Use a Buffer as a vertex-buffer
- ```bindIndexBuffer(Buffer buffer)```Use a Buffer as an index-buffer
- ```bindInputSet(InputSet inputSet)```Bind all Bindings specified in the InputSet 
- ```draw(uint32_t vertexCount, uint32_t firstVertex)```Issue a draw command with the number of vertices and an offset into the currently bound vertex-buffer
- ```drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)```Issue am indexed draw command with the number of indices, an offset into the currently bound index-buffer and an offset into the currently bound vertex-buffer

To execute a CommandBuffer call ```Interface::execute(CommandBuffer commandBuffer)```

The handle to a CommandBuffer is valid until a call to ```Interface::free(CommandBuffer commandBuffer);``` or until the descruction of the interface



