#ifndef WINDOW_H
#define WINDOW_H
#include <iostream>
#include <functional>
#include "metal_imports.hpp"
#include "renderer.hpp"

class MTKViewDelegate : public MTK::ViewDelegate {
public:
    MTKViewDelegate(MTL::Device* device, MTK::View* view);
//    ~MTKViewDelegate();
    void drawInMTKView(MTK::View* view) override;
private:
    MTK::View* view;
    std::unique_ptr<Renderer> renderer;
};

class AppDelegate : public NS::ApplicationDelegate {
public:
    ~AppDelegate();
    
    void applicationWillFinishLaunching(NS::Notification* notification) override;
    void applicationDidFinishLaunching(NS::Notification* notification) override;
    bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;
private:
    NS::Window* window;
    MTK::View* view;
    MTL::Device* device;
    std::unique_ptr<MTKViewDelegate> viewDelegate;
};

#endif
