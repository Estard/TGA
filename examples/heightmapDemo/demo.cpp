#include "HeightmapViewer.hpp"
#include "hm.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

struct RGBA{
    uint8_t r,g,b,a;
};

constexpr RGBA green {0x56,0x7d,0x46,0x96};
constexpr RGBA lgreen {0x5f,0x89,0x4c,0x96};
constexpr RGBA dgreen {0x38,0x51,0x2d,0x96};

std::vector<RGBA> grassTex = {
green,lgreen,dgreen,lgreen,
dgreen,lgreen,lgreen,green,
lgreen,green,dgreen,green,
green,green,green,green
};


std::tuple<std::vector<uint8_t>,uint32_t, uint32_t> loadTex(const std::string &file)
{
    int w,h,channels;
    uint8_t* p = stbi_load(file.c_str(),&w,&h,&channels,STBI_rgb_alpha);
    if(!p)
        return {};
    return {std::vector<uint8_t>(p,p+w*h*4),w,h};
}


int main(){
    try
    {
        HeightmapViewer hv;
        auto hm = ::hm();
        float terrainHeight = hm.width;
        hv.setHeightmap(hm.data.data(),hm.width,hm.height,10,10,terrainHeight*1.25);
        /*{
            auto [tex, w, h] = loadTex("someGrassyTexture.png");
            hv.setSurfaceLowTexture(tex.data(),w,h);
        }

        {
            auto [tex, w, h] = loadTex("someDirtyTexture.png");
            hv.setSideLowTexture(tex.data(),w,h);
        }

        {
            auto [tex, w, h] = loadTex("someSnowyTexture.png");
            hv.setSurfaceHighTexture(tex.data(),w,h);
        }

        {
            auto [tex, w, h] = loadTex("someRockTexture.png");
            hv.setSideHighTexture(tex.data(),w,h);
        }
        hv.setTextureTiling(4./hm.width,4./hm.height);*/
        hv.view();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}