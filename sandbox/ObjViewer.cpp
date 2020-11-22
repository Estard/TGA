#include "Framework.hpp"
#include "tga/tga_math.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "stb_image.h"




struct Vertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    bool operator==(const Vertex &other) const{
        return position == other.position && uv == other.uv && normal == other.normal;
    }
};


struct Camera
{
    alignas(16) glm::mat4 view = glm::mat4(1);
    alignas(16) glm::mat4 projection = glm::mat4(1);
    alignas(16) glm::vec3 lightPos = glm::vec3(0);
};

struct Transform
{
    alignas(16) glm::mat4 transform = glm::mat4(1);
};




namespace std
{
    template<> struct hash<Vertex>{
        size_t operator()(Vertex const &v) const{
            return ((hash<glm::vec3>()(v.position)^
                    (hash<glm::vec3>()(v.normal)<<1))>>1)^
                    (hash<glm::vec2>()(v.uv)<<1);
        }
    };
}

class ObjViewer: public Framework
{
   
    public:
    ObjViewer(std::string const &_objFilepath,
         std::string const &_diffuseMapPath = "",
         std::string const &_normalMapPath = "")
         : objFilepath(_objFilepath), diffuseMapPath(_diffuseMapPath),normalMapPath(_normalMapPath)
    {}

    private:
    void OnCreate()
    {
        tgai->beginCommandBuffer();
        cmdBuffer = tgai->endCommandBuffer();
        LoadObj();
        LoadData();
        CreateRenderPass();
        CreateInputSets();
        
    }
    void OnUpdate(uint32_t frame)
    {
        UpdateCam();
        tgai->beginCommandBuffer(cmdBuffer);
        
        tgai->bindVertexBuffer(vertexBuffer);
        tgai->bindIndexBuffer(indexBuffer);

        tgai->setRenderPass(renderPass,frame);
        tgai->bindInputSet(camInputSet);
        if(diffuseMapInputSet)
            tgai->bindInputSet(diffuseMapInputSet);
        if(normalMapInputSet)
            tgai->bindInputSet(normalMapInputSet);
        tgai->drawIndexed(modelVertexCount,0,0);
        cmdBuffer = tgai->endCommandBuffer();
        tgai->execute(cmdBuffer);
    }
    void OnDestroy()
    {
       //Nobody cleans up their memory anyway
       //...
       //Also the Destructor of tgai does that anyway
    }

    void LoadObj()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if(!tinyobj::LoadObj(&attrib,&shapes,&materials,&warn,&err,objFilepath.c_str()))
            throw std::runtime_error(warn+err);
        
        std::vector<Vertex> preVertexBuffer;
        for(const auto &shape: shapes){
            for(const auto &index: shape.mesh.indices){
                Vertex vertex{};
                vertex.position = {
                    attrib.vertices[3*index.vertex_index+0],
                    attrib.vertices[3*index.vertex_index+1],
                    attrib.vertices[3*index.vertex_index+2]};
                vertex.normal = {
                    attrib.normals[3*index.normal_index+0],
                    attrib.normals[3*index.normal_index+1],
                    attrib.normals[3*index.normal_index+2]};
                vertex.uv = {
                    attrib.texcoords[2*index.texcoord_index+0],
                    1.f-attrib.texcoords[2*index.texcoord_index+1]
                };
                preVertexBuffer.emplace_back(vertex);

                circleRadius = std::max(circleRadius,vertex.position.x);
                circleRadius = std::max(circleRadius,vertex.position.y);
                circleRadius = std::max(circleRadius,vertex.position.z);
            }
        }

        std::unordered_map<Vertex, uint32_t> foundVertices{};
        std::vector<Vertex> vBuffer;
        std::vector<uint32_t> iBuffer;


        // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/
        for(size_t i = 0; i <  preVertexBuffer.size(); i+=3){
            auto &p0 = preVertexBuffer[i+0].position;
            auto &p1 = preVertexBuffer[i+1].position;
            auto &p2 = preVertexBuffer[i+2].position;

            auto &uv0 = preVertexBuffer[i+0].uv;
            auto &uv1 = preVertexBuffer[i+1].uv;
            auto &uv2 = preVertexBuffer[i+2].uv;

            auto deltaPos1 = p1-p0;
            auto deltaPos2 = p2-p0;

            auto deltaUV1 = uv1-uv0;
            auto deltaUV2 = uv2-uv0;

            float r = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV1.y *deltaUV2.x);
            glm::vec3 tangent = (deltaPos1*deltaUV2.y -deltaPos2*deltaUV1.y)*r;
            glm::vec3 bitangent = (deltaPos2*deltaUV1.x - deltaPos1*deltaUV2.x)*r;

            preVertexBuffer[i+0].tangent = tangent;
            preVertexBuffer[i+1].tangent = tangent;
            preVertexBuffer[i+2].tangent = tangent;

            preVertexBuffer[i+0].bitangent = bitangent;
            preVertexBuffer[i+1].bitangent = bitangent;
            preVertexBuffer[i+2].bitangent = bitangent;
        }

        for(const auto& vertex: preVertexBuffer){
            if(!foundVertices.count(vertex)){//It's a new Vertex
                foundVertices[vertex] = static_cast<uint32_t>(vBuffer.size());
                vBuffer.emplace_back(vertex);
            }
            else{//Seen before, average the the tangents 
                auto &v = vBuffer[foundVertices[vertex]];
                v.tangent += vertex.tangent;
                v.bitangent += vertex.bitangent;
            }
            iBuffer.emplace_back(foundVertices[vertex]);
        }
        preVertexBuffer.clear();
        modelVertexCount = static_cast<uint32_t>(iBuffer.size());
        uint32_t vBufferSize = vBuffer.size()*sizeof(Vertex);
        uint32_t iBufferSize = iBuffer.size()*sizeof(uint32_t);

        vertexBuffer = tgai->createBuffer({tga::BufferUsage::vertex,(uint8_t*)vBuffer.data(),vBufferSize});
        indexBuffer = tgai->createBuffer({tga::BufferUsage::index,(uint8_t*)iBuffer.data(),iBufferSize});
    }

    void LoadData()
    {
        auto loadTex = [&](tga::Texture &tex, std::string const & filepath, tga::Format format){
            if(filepath.size()>0){
                int width, height,channels;
            stbi_uc* pixels  = stbi_load(filepath.c_str(),&width,&height,&channels,STBI_rgb_alpha);
            if(pixels){
                tex = tgai->createTexture({
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height),
                    format,
                    static_cast<uint8_t*>(pixels),width*height*4u,
                    tga::SamplerMode::linear});
            }
        }};
        loadTex(diffuseMap,diffuseMapPath, tga::Format::r8g8b8a8_srgb);
        loadTex(normalMap,normalMapPath, tga::Format::r8g8b8a8_unorm);
        camData = tgai->createBuffer({tga::BufferUsage::uniform, (uint8_t*)&cam, sizeof(cam)});
        modelData = tgai->createBuffer({tga::BufferUsage::uniform, (uint8_t*)&modelTransform,sizeof(modelTransform)});
    }

    void CreateRenderPass()
    {
        auto LoadShader = [&](const std::string& filename, tga::ShaderType type){
            std::ifstream file(filename, std::ios::ate | std::ios::binary);
            if (!file.is_open()) 
                throw std::runtime_error("failed to open file " +filename);
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> shaderData(fileSize);
            file.seekg(0);
            file.read(shaderData.data(), fileSize);
            file.close();
            return tgai->createShader({type,(uint8_t*)shaderData.data(),shaderData.size()});
        };

        tga::InputLayout inputLayout({
            //Set = 0: Camera Data, Object Data
            {{{tga::BindingType::uniformBuffer},{tga::BindingType::uniformBuffer}}},
            //Set = 1: Diffuse Map
            {{{tga::BindingType::sampler2D}}},
            //Set = 2: Normal Map
            {{{tga::BindingType::sampler2D}}}});

        
        tga::Shader vs,fs;

        if(diffuseMap && normalMap){
            vs = LoadShader("shaders/objDNVert.spv",tga::ShaderType::vertex);
            fs = LoadShader("shaders/objDNFrag.spv",tga::ShaderType::fragment);
        }
        else if(diffuseMap){
            vs = LoadShader("shaders/objVert.spv",tga::ShaderType::vertex);
            fs = LoadShader("shaders/objDFrag.spv",tga::ShaderType::fragment);

        }else{
            vs = LoadShader("shaders/objVert.spv",tga::ShaderType::vertex);
            fs = LoadShader("shaders/objFrag.spv",tga::ShaderType::fragment);
        }  

        renderPass = tgai->createRenderPass({
                {vs,fs},_frameworkWindow,tga::ClearOperation::all,
                {tga::CompareOperation::less,false,tga::BlendFactor::srcAlpha,tga::BlendFactor::oneMinusSrcAlpha,
                    tga::FrontFace::counterclockwise, tga::CullMode::none},
                inputLayout,
                { //Vertex Layout
                    sizeof(Vertex), {
                    {offsetof(Vertex,position), tga::Format::r32g32b32_sfloat},
                    {offsetof(Vertex,uv), tga::Format::r32g32_sfloat},
                    {offsetof(Vertex,normal), tga::Format::r32g32b32_sfloat},
                    {offsetof(Vertex,tangent), tga::Format::r32g32b32_sfloat},
                    {offsetof(Vertex,bitangent), tga::Format::r32g32b32_sfloat}}
                }
            });
    }

    void CreateInputSets()
    {
        camInputSet = tgai->createInputSet({renderPass,0,{{camData,0},{modelData,1}}});
        if(diffuseMap)
            diffuseMapInputSet = tgai->createInputSet({renderPass,1,{{diffuseMap,0}}});
        if(normalMap)
            normalMapInputSet = tgai->createInputSet({renderPass,2,{{normalMap,0}}});
    }


    void UpdateCam()
    {
        static float fullTime = 0.0;
        fullTime +=deltaTime;
        const glm::vec3 position = glm::vec3(1.5*circleRadius*std::sin(fullTime),2.f,1.5*circleRadius*std::cos(fullTime));
        //const glm::vec3 lookDirection = glm::vec3(1.f,0.f,0.f);
        const glm::vec3 up = glm::vec3(0.f,1.f,0.f);

        cam.projection = glm::perspective(glm::radians(90.f),
            _frameworkWindowWidth/static_cast<float>(_frameworkWindowHeight),
            0.1f,1000.f);
        cam.projection[1][1] *= -1;
        cam.view = glm::lookAt(position,glm::vec3(0,0,0),up);
        cam.lightPos = glm::vec3(2*circleRadius);
        tgai->updateBuffer(camData,(uint8_t*)&cam,sizeof(cam),0);
    }

    std::string objFilepath;
    std::string diffuseMapPath;
    std::string normalMapPath;
    tga::CommandBuffer cmdBuffer;
    tga::Buffer vertexBuffer;
    tga::Buffer indexBuffer;

    tga::Buffer camData;
    tga::Buffer modelData;

    tga::Texture diffuseMap;
    tga::Texture normalMap;

    tga::RenderPass renderPass;
    tga::InputSet camInputSet, diffuseMapInputSet, normalMapInputSet;
    
    Camera cam;
    Transform modelTransform;
    uint32_t modelVertexCount;
    float circleRadius = 1;
};

int main(int argc, char **argv)
{
    try
    {
        switch (argc)
        {
            case 1:{
                std::cerr << "No Files provided\n";
            }
                break;
            case 2:{
                ObjViewer objViewer(argv[1]);
                objViewer.run();
            }
                break;
            case 3:{
                ObjViewer objViewer(argv[1],argv[2]);
                objViewer.run();
            }
                break;

            default:{
                ObjViewer objViewer(argv[1],argv[2],argv[3]);
                objViewer.run();

            }
            break;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "Done" << std::endl;
    
}