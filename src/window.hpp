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
#include <mutex>
#include <condition_variable>
#include <thread>

struct HandlerState {
    std::function<void(int keyCode, runtime::Modifiers modifiers)> keyDownHandler;
    std::function<void(int keyCode, runtime::Modifiers modifiers)> keyUpHandler;
    std::function<void(float x, float y, runtime::MouseButton button, runtime::Modifiers modifiers)> mouseDownHandler;
    std::function<void(float x, float y, runtime::MouseButton button, runtime::Modifiers modifiers)> mouseUpHandler;
    std::function<void(float x, float y, runtime::Modifiers modifiers)> mouseMovedHandler;
    std::function<void(float dx, float dy, float x, float y, runtime::Modifiers modifiers)> scrollWheelHandler;
};


class MTKViewDelegate : public MTK::ViewDelegate {
public:
    MTKViewDelegate(MTL::Device* device, MTK::View* view);
    ~MTKViewDelegate();

    void resizeWatcher();

    void drawInMTKView(MTK::View* view) override;
    void drawableSizeWillChange(MTK::View* view, CGSize frameSize) override;
    MTK::View* view;
    std::unique_ptr<Renderer> renderer;
    std::condition_variable cv;
    std::mutex m;
    bool ready;
    std::thread resizeThread;
};

class AppDelegate : public NS::ApplicationDelegate {
public:
    ~AppDelegate();
    
    void applicationWillFinishLaunching(NS::Notification* notification) override;
    void applicationDidFinishLaunching(NS::Notification* notification) override;
    bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;
    void setFocused(tree::TreeNode* node);
    tree::TreeNode* hitTest(float x, float y, simd_float2& testPoint);
    
    NS::Window* window;
    MTK::View* view;
    MTL::Device* device;
    tree::TreeNode* focused = nullptr;
    tree::TreeNode* hovered = nullptr;
    tree::TreeNode* mouseDownTarget = nullptr;
    std::unique_ptr<MTKViewDelegate> viewDelegate;
};
