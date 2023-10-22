#pragma once
#include "tga/tga.hpp"
#include "tga/tga_vulkan/tga_vulkan.hpp"
#include <thread>
#include <chrono>

class Framework
{
    public:
    Framework():deltaTime(0.016),tgai(){}
    
    void run(uint32_t width = 0, uint32_t height = 0, bool vSyncOn = true)
    {
        if(!width || !height){
            auto [w,h] = tgai.screenResolution();
            width = w;
            height = h;
        }
        _frameworkWindowWidth = width;
        _frameworkWindowHeight = height;
        using namespace std::chrono_literals;
        _frameworkWindow = tgai.createWindow({width,height,vSyncOn?tga::PresentMode::vsync:tga::PresentMode::immediate});
        OnCreate();
        auto beginTime = std::chrono::steady_clock::now();
        while(!tgai.windowShouldClose(_frameworkWindow))
        {
            auto nextFrame = tgai.nextFrame(_frameworkWindow);
            OnUpdate(nextFrame);
            tgai.present(_frameworkWindow,nextFrame);
            auto endTime = std::chrono::steady_clock::now();
            deltaTime = std::chrono::duration<double>(endTime-beginTime).count();
            beginTime = endTime;
        }
        OnDestroy();
    }

    private:
    virtual void OnCreate(){}
    virtual void OnUpdate(uint32_t){}
    virtual void OnDestroy(){}

    protected:
    double deltaTime;
    tga::Interface tgai;
    tga::Window _frameworkWindow;
    uint32_t _frameworkWindowWidth,_frameworkWindowHeight;
};