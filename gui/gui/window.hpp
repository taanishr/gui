//
//  window.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once
#include <iostream>
#include <functional>
#include "metal_imports.hpp"
#include "renderer.hpp"
#include <ApplicationServices/ApplicationServices.h>
#include <objc/runtime.h>
#include "events.hpp"

struct HandlerState {
    std::function<void(char ch)> keyboardHandler;
    std::function<void(float x, float y)> mouseDownHandler;
    std::function<void()> resizeHandler;
};


class MTKViewDelegate : public MTK::ViewDelegate {
public:
    MTKViewDelegate(MTL::Device* device, MTK::View* view);
    void drawInMTKView(MTK::View* view) override;
    void drawableSizeWillChange(MTK::View* view, CGSize frameSize) override;
    MTK::View* view;
    std::unique_ptr<Renderer> renderer;
};

class AppDelegate : public NS::ApplicationDelegate {
public:
    ~AppDelegate();
    
    void applicationWillFinishLaunching(NS::Notification* notification) override;
    void applicationDidFinishLaunching(NS::Notification* notification) override;
    bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;
    
    NS::Window* window;
    MTK::View* view;
    MTL::Device* device;
    NewArch::TreeNode* focused = nullptr;
    std::unique_ptr<MTKViewDelegate> viewDelegate;
};
