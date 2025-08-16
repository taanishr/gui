//
//  window.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#include "window.hpp"
#include <AppKit_Extensions-Swift.h>

extern "C" bool acceptsFirstResponder(id self, SEL _cmd) {
    return true;
}

using KeyDownFunc = NS::String*(*)(id, SEL);

extern "C" void keyDown(id self, SEL _cmd, id event) {
    KeyDownFunc f = (KeyDownFunc)objc_msgSend; // must cast the send function
    auto chars = f(event, sel_registerName("characters")); // in obj-c, values are basically methods too
    
    std::string inputStr = chars->cString(NS::UTF8StringEncoding);
    
    if (inputStr == "\x7F" && selectedString.length() > 0)
        selectedString.pop_back();
    else
        selectedString += inputStr;
}

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
    CGRect frame = CGRect{{100.0, 100.0}, {windowHeight, windowWidth}};
    
    window = NS::Window::alloc()->init(
                frame,
                NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled|NS::WindowStyleMaskMiniaturizable|NS::WindowStyleMaskResizable,
                NS::BackingStoreBuffered,
                false
             );
    
    device = MTL::CreateSystemDefaultDevice();

    view = MTK::View::alloc()->init(frame, device);
    
    
    // adding input!!!
    id objcInstance = reinterpret_cast<id>(view);
    Class cls = object_getClass(objcInstance);
    
    class_addMethod(cls, sel_registerName("acceptsFirstResponder"),
                    reinterpret_cast<IMP>(acceptsFirstResponder), "B@:");
    
    class_addMethod( cls , sel_registerName("keyDown:"), reinterpret_cast<IMP>(keyDown), "v@:@");
    

    view->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
//    view->setClearColor(MTL::ClearColor::Make(0.33,0.28,0.78,0.3));
    view->setClearColor(MTL::ClearColor::Make(0,0,0,0.3));
    view->setDepthStencilPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float);
    AppKit_Extensions::setMaximumDrawableCount(reinterpret_cast<void*>(view), 2);

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
