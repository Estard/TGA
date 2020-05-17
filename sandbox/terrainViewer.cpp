#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"
#include "tga/tga_math.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "util.hpp"


struct Configuration{
    std::string heightmap;
    std::string normalmap;
    std::string colormap;
    std::string vertShader = "shaders/TerrainViewerVert.spv";
    std::string fragShader = "shaders/TerrainViewerFrag.spv";
};

struct PixelRGB{
    uint8_t r,g,b;
};

struct ImageRGB{
    uint32_t width,height;
    std::vector<PixelRGB> pixels;
};

ImageRGB loadImage(const std::string file)
{
	int texWidth, texHeight, texChannels;
    	stbi_uc* pixels = stbi_load(file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb);
	if (!pixels) 
    	throw std::runtime_error("[TerrainViewer] Error: failed to load image! " + file);
	
	PixelRGB *data = reinterpret_cast<PixelRGB*>(pixels);
	size_t len = texWidth*texHeight;
	ImageRGB i = {static_cast<uint32_t>(texWidth),static_cast<uint32_t>(texHeight),std::vector<PixelRGB>(data,data+len)};
	stbi_image_free(pixels);
	return i;
}


Configuration parseArgs(int argc, char** argv)
{
    Configuration config{};
    for(int i = 1; i < argc; i++){
        char c = argv[i][0];
        if(c!='-')
            continue;
        std::string argument(&argv[i][1]);
        if((i+1)==argc){
           std::cerr << "[TerrainViewer] Warning: option without parameter: "<< argument<<'\n';  
           break;
        }
        std::string parameter(argv[i+1]);

        if(argument == "h" || argument == "heightmap")
            config.heightmap = parameter;
        else if(argument == "n" || argument == "normalmap")
            config.normalmap = parameter;
        else if(argument == "c" || argument == "colormap")
            config.colormap = parameter;
        else if(argument == "vert")
            config.vertShader = parameter;
        else if(argument == "frag")
            config.fragShader = parameter;
        else if(argument == "help"||argument == "-help")
            std::cerr << "[TerrainViewer] Usage: ./terrainViewer -h heightmap -n normalmap -c colormap\n";
        else
            std::cerr << "[TerrainViewer] Warning: unknown argumente: " << argument <<" consumes parameter "<< parameter<<'\n';
        
        
    }
    if(!config.heightmap.length())
        throw std::runtime_error("[TerrainViewer] Error: Missing argument -heightmap");
    if(!config.normalmap.length())
        throw std::runtime_error("[TerrainViewer] Error: Missing argument -normalmap");
    if(!config.colormap.length())
        throw std::runtime_error("[TerrainViewer] Error: Missing argument -colormap");

    return config;
}

class TerrainViewer{
    Configuration config;
    tga::TGAVulkan tgav;
    
    ImageRGB heightmap;
    ImageRGB normalmap;
    ImageRGB colormap;
    tga::Texture heightmapTex;
    tga::Texture normalmapTex;
    tga::Texture colormapTex;
    tga::Window window;
    tga::Shader vertShader;
    tga::Shader fragShader;
    std::vector<uint32_t> indexBufferData;
    tga::Buffer indexBuffer;

    struct Data{
        alignas(16) float time = 0.0;
        alignas(16) glm::vec3 terrainSize = glm::vec3(1);
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    }data;

    public:
        TerrainViewer(const Configuration &_config):
            config(_config),tgav(tga::TGAVulkan())
            {
                loadImages();
                uploadTextures();
                createIndexBuffer();
            }
        void loadImages()
        {
            heightmap = loadImage(config.heightmap);
            normalmap = loadImage(config.normalmap);
            colormap = loadImage(config.colormap);
        }
        void uploadTextures()
        {
            heightmapTex = tgav.createTexture({heightmap.width,heightmap.height,tga::Format::r8g8b8_unorm,
            (uint8_t*)heightmap.pixels.data(),heightmap.pixels.size()*sizeof(PixelRGB),tga::SamplerMode::linear,tga::RepeatMode::clampEdge});
            normalmapTex = tgav.createTexture({normalmap.width,normalmap.height,tga::Format::r8g8b8_unorm,
            (uint8_t*)normalmap.pixels.data(),normalmap.pixels.size()*sizeof(PixelRGB),tga::SamplerMode::linear,tga::RepeatMode::clampEdge});
            colormapTex = tgav.createTexture({colormap.width,colormap.height,tga::Format::r8g8b8_unorm,
            (uint8_t*)colormap.pixels.data(),colormap.pixels.size()*sizeof(PixelRGB),tga::SamplerMode::linear,tga::RepeatMode::repeate});
        }
        void createIndexBuffer(){
            uint32_t width = heightmap.width;
            uint32_t height = heightmap.height;
            for(uint32_t y = 0; y < height-1; y++)
                for(uint32_t x = 0; x < width-1; x++){
                    indexBufferData.push_back(x+y*width);
                    indexBufferData.push_back((x+1)+y*width);
                    indexBufferData.push_back(x+(y+1)*width);

                    indexBufferData.push_back(x+(y+1)*width);
                    indexBufferData.push_back((x+1)+y*width);
                    indexBufferData.push_back((x+1)+(y+1)*width);
                }
            indexBuffer = tgav.createBuffer({tga::BufferUsage::index,(uint8_t*)indexBufferData.data(),
            indexBufferData.size()*sizeof(indexBufferData[0])});
        }
        void show()
        {
            data.terrainSize = glm::vec3(float(heightmap.width),float(heightmap.height),.25*(heightmap.width+heightmap.height));
            glm::vec3 camPos = data.terrainSize*glm::vec3(.5f,.5f,2.f);
            data.view = glm::lookAt(camPos,glm::vec3(0),glm::vec3(0,0,1));
            data.projection = glm::perspective(glm::radians(60.f),1600/float(900),0.1f,5000.f);
            data.projection[1][1] *= -1;

            vertShader = loadShader(tgav,config.vertShader,tga::ShaderType::vertex);
            fragShader = loadShader(tgav,config.fragShader,tga::ShaderType::fragment);
            window = tgav.createWindow({1600,900,tga::PresentMode::vsync});

            tga::RenderPassInfo rpInfo = {{vertShader,fragShader},window};
            rpInfo.inputLayout.setLayouts.push_back({{{tga::BindingType::sampler2D},{tga::BindingType::sampler2D},
                {tga::BindingType::sampler2D},{tga::BindingType::uniformBuffer}}});
            rpInfo.rasterizerConfig.frontFace = tga::FrontFace::counterclockwise;
            rpInfo.rasterizerConfig.depthCompareOp = tga::CompareOperation::lessEqual;
            rpInfo.rasterizerConfig.cullMode = tga::CullMode::back;
            //rpInfo.rasterizerConfig.polygonMode = tga::PolygonMode::wireframe;
            rpInfo.clearOperations = tga::ClearOperation::all;
            auto rp = tgav.createRenderPass(rpInfo);
            auto ubo = tgav.createBuffer({tga::BufferUsage::uniform,(uint8_t*)&data,sizeof(data)});

            auto inputSet = tgav.createInputSet({rp,0,{{heightmapTex,0},{normalmapTex,1},{colormapTex,2},{ubo,3}}});
            std::vector<tga::CommandBuffer> cmdBuffers{};
            for(uint32_t i = 0; i < tgav.backbufferCount(window);i++){
                tgav.beginCommandBuffer();
                tgav.bindIndexBuffer(indexBuffer);
                tgav.setRenderPass(rp,i);
                tgav.bindInputSet(inputSet);
                tgav.drawIndexed(indexBufferData.size(),0,0);
                cmdBuffers.emplace_back(tgav.endCommandBuffer());
            }
            Timer t;
            while(!tgav.windowShouldClose(window)){
                auto dt = t.deltaTime();
                data.time += dt;
                t.reset();
                tgav.updateBuffer(ubo,(uint8_t*)&data,sizeof(data),0);
                tgav.execute(cmdBuffers[tgav.nextFrame(window)]);
                tgav.present(window);
            }
        }
};




int main(int argc, char** argv)
{  
    std::cout << "Viewing Terrain\n";
    try
    {
        TerrainViewer tv(parseArgs(argc,argv));
        tv.show();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Finished Viewing\n";
    return 0;
}