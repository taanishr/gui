//
//  window.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#include "window.hpp"
#include <AppKit_Extensions-Swift.h>


MTKViewDelegate::MTKViewDelegate(MTL::Device* device, MTK::View* view):
    view{view},
    renderer{new Renderer{device, view}}
{}

void MTKViewDelegate::drawInMTKView(MTK::View* view)
{
     renderer->draw();
}


AppDelegate::~AppDelegate()
{
    window->release();
    view->release();
    device->release();
}

void AppDelegate::applicationWillFinishLaunching(NS::Notification* notification)
{
    NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
    app->setActivationPolicy(NS::ActivationPolicyRegular);
}

void AppDelegate::applicationDidFinishLaunching(NS::Notification* notification)
{
    CGRect frame = CGRect{{100.0, 100.0}, {512.0, 512.0}};
    
    window = NS::Window::alloc()->init(
                frame,
                NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled,
                NS::BackingStoreBuffered,
                false
             );
    
    device = MTL::CreateSystemDefaultDevice();
    
    view = MTK::View::alloc()->init(frame, device);

    view->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
//    view->setClearColor(MTL::ClearColor::Make(0.33,0.28,0.78,0.3));
    view->setClearColor(MTL::ClearColor::Make(1,1,1,1));
    view->setDepthStencilPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float);

    viewDelegate = std::unique_ptr<MTKViewDelegate>(new MTKViewDelegate{device, view});
    view->setDelegate(viewDelegate.get());

    window->setContentView(view);

    window->setTitle(NS::String::string("window", NS::StringEncoding::UTF8StringEncoding ));

    window->makeKeyAndOrderFront(nullptr);
    
    AppKit_Extensions::setTitleBarTransparent(reinterpret_cast<void*>(window));
    AppKit_Extensions::setWindowTransparent(reinterpret_cast<void*>(window));
    
    NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
    
    app->activateIgnoringOtherApps(true);
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender)
{
    return true;
}
