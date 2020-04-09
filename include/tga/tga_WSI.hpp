#pragma once

#include "tga.hpp"

namespace tga{
    class WSI{
        public:
        WSI()=default;
        virtual Window createWindow(const WindowInfo &windowInfo) = 0;
        virtual void free(Window window) = 0;
        virtual void setWindowTitle(Window window, const char* title) = 0;
        virtual uint32_t aquireNextImage(Window window) = 0;
        virtual void presentImage(Window window) = 0;
        virtual bool windowShouldClose(Window window) = 0;
        virtual bool keyDown(Window window, Key key) = 0;
        virtual std::pair<int, int> mousePosition(Window window) = 0;
    };
};