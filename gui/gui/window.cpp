//
//  window.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//



#include "window.hpp"
#include "AppKit_Extensions.hpp"
#include "events.hpp"
#include "renderer.hpp"
#include "renderer_constants.hpp"
#include "index.hpp"
#include <print>
#include "metal_imports.hpp"

// refactor objective c into a cleaner interface? maybe a struct with key funcs?
HandlerState hs {};

using KeyDownFunc = NS::String*(*)(id, SEL);
using MouseDownFunc = CGPoint(*)(id, SEL);


extern "C" bool acceptsFirstResponder(id self, SEL _cmd) {
    return true;
}

extern "C" void keyDown(id self, SEL _cmd, id event) {
    KeyDownFunc f = (KeyDownFunc)objc_msgSend;
    auto chars = f(event, sel_registerName("characters"));
    
    char inputChar = chars->cString(NS::UTF8StringEncoding)[0];

    std::println("writing...");
    
    hs.keyboardHandler(inputChar);
}

extern "C" void mouseDown(id self, SEL _cmd, id event) {
    MouseDownFunc f = (MouseDownFunc)objc_msgSend;
    auto point = f(event, sel_registerName("locationInWindow"));
    
    hs.mouseDownHandler(point.x, point.y);
}


MTKViewDelegate::MTKViewDelegate(MTL::Device* device, MTK::View* view):
    view{view},
    renderer{new Renderer{device, view}}
{
    renderer->makeCurrent();
}

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
                NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled|NS::WindowStyleMaskMiniaturizable|NS::WindowStyleMaskResizable,
                NS::BackingStoreBuffered,
                false
             );
    
    
    device = MTL::CreateSystemDefaultDevice();

    view = MTK::View::alloc()->init(frame, device);
    

    //    view->setClearColor(MTL::ClearColor::Make(0.33,0.28,0.78,0.3));
        view->setClearColor(MTL::ClearColor::Make(0,0,0,0.3));
    //    view->setClearColor(MTL::ClearColor::Make(0.33,0.33,0.33,0.33));
    
    view->setColorPixelFormat(MTL::PixelFormat::PixelFormatRGBA8Unorm);
//    view->setClearColor(MTL::ClearColor::Make(0,0,0,1));
    view->setDepthStencilPixelFormat(MTL::PixelFormat::PixelFormatDepth32Float);
    AppKit_Extensions::setMaximumDrawableCount(reinterpret_cast<void*>(view), MaxOutstandingFrameCount);
    
    viewDelegate = std::make_unique<MTKViewDelegate>(device,view);
    view->setDelegate(viewDelegate.get());
    
    // adding input!!!
    id objcInstance = reinterpret_cast<id>(view);
    Class cls = object_getClass(objcInstance);

    
    hs.keyboardHandler = [this](char ch){
        Event e;
        e.type = EventType::KeyDown;
        e.payload = KeyboardPayload{
            .keyCode = static_cast<int>(ch)
        };

        // auto currTreeHandle = TreeManager::getCurrentTreeHandle();
        // auto currTree = TreeManager::getTree(currTreeHandle);
        // currTree->getRoot()->dispatch(e);

        if (this->focused) {
            this->focused->dispatch(e);
        }
    };
    
    hs.mouseDownHandler = [this](float x, float y){
        auto frameInfo = this->viewDelegate->renderer->getFrameInfo();
        auto testPoint = simd_float2{x,frameInfo.height - y};

        Event e;
        e.type = EventType::MouseDown;
        // std::println("mouse down click!");
        e.type = EventType::MouseDown;
        e.payload = MousePayload{
            .position = testPoint,
            .button = MouseButton::Left
        };

        auto currTreeHandle = TreeManager::getCurrentTreeHandle();
        auto currTree = TreeManager::getTree(currTreeHandle);
        auto root = currTree->getRoot();
        auto htnode = currTree->hitTestRecursive(root, testPoint);

        if (!htnode)
            return;      
        
        this->focused = htnode;
        htnode->dispatch(e);
    };
    
    class_addMethod(cls, sel_registerName("acceptsFirstResponder"),
                    reinterpret_cast<IMP>(acceptsFirstResponder), "B@:");
    class_addMethod( cls , sel_registerName("keyDown:"), reinterpret_cast<IMP>(keyDown), "v@:@");
    class_addMethod( cls , sel_registerName("mouseDown:"), reinterpret_cast<IMP>(mouseDown), "v@:@");
    
    
    // run some tests (no visual updates)
    // UIContext ctx {this->device, this->view};
    // index(ctx);

    window->setContentView(view);
    
    window->setTitle(NS::String::string("window", NS::StringEncoding::UTF8StringEncoding ));

    window->makeKeyAndOrderFront(nullptr);
    
    AppKit_Extensions::setTitleBarTransparent(reinterpret_cast<void*>(window));
    // AppKit_Extensions::setWindowTransparent(reinterpret_cast<void*>(window));
    

    NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
    
    app->activateIgnoringOtherApps(true);
    
}

bool AppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender)
{
    return true;
}
