//
//  window.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//



#include "window.hpp"
#include "AppKit_Extensions.hpp"
#include "events.hpp"
#include "context_manager.hpp"
#include "renderer.hpp"
#include "renderer_constants.hpp"
#include "index.hpp"
#include <CoreFoundation/CFCGTypes.h>
#include <memory>
#include <objc/message.h>
#include <objc/objc.h>
#include <print>
#include "metal_imports.hpp"

// refactor objective c into a cleaner interface? maybe a struct with key funcs?
HandlerState hs {};

using runtime::ContextManager;
using runtime::Event;
using runtime::EventType;
using runtime::FocusPayload;
using runtime::KeyboardPayload;
using runtime::Modifiers;
using runtime::MouseButton;
using runtime::MousePayload;
using runtime::ScrollPayload;
using tree::DirtyBits;

using KeyCodeFunc = unsigned short(*)(id, SEL);
using LocationFunc = CGPoint(*)(id, SEL);
using ScrollWheelFunc = CGFloat(*)(id, SEL);
using ModifierFlagsFunc = unsigned long(*)(id, SEL);
using ButtonNumberFunc = long(*)(id, SEL);

namespace {

constexpr unsigned long NSEventModifierFlagShift = 1UL << 17;
constexpr unsigned long NSEventModifierFlagControl = 1UL << 18;
constexpr unsigned long NSEventModifierFlagOption = 1UL << 19;
constexpr unsigned long NSEventModifierFlagCommand = 1UL << 20;

Modifiers modifiersFromEvent(id event) {
    ModifierFlagsFunc f = reinterpret_cast<ModifierFlagsFunc>(objc_msgSend);
    auto flags = f(event, sel_registerName("modifierFlags"));

    return Modifiers{
        .shift = (flags & NSEventModifierFlagShift) != 0,
        .ctrl = (flags & NSEventModifierFlagControl) != 0,
        .alt = (flags & NSEventModifierFlagOption) != 0,
        .meta = (flags & NSEventModifierFlagCommand) != 0
    };
}

MouseButton mouseButtonFromEvent(id event) {
    ButtonNumberFunc f = reinterpret_cast<ButtonNumberFunc>(objc_msgSend);
    auto buttonNumber = f(event, sel_registerName("buttonNumber"));

    if (buttonNumber == 1) return MouseButton::Right;
    if (buttonNumber == 2) return MouseButton::Middle;
    return MouseButton::Left;
}

simd_float2 toViewPoint(float x, float y, float height) {
    return simd_float2{x, height - y};
}

}

extern "C" bool acceptsFirstResponder(id self, SEL _cmd) {
    return true;
}

extern "C" void keyDown(id self, SEL _cmd, id event) {
    KeyCodeFunc f = reinterpret_cast<KeyCodeFunc>(objc_msgSend);
    hs.keyDownHandler(static_cast<int>(f(event, sel_registerName("keyCode"))), modifiersFromEvent(event));
}

extern "C" void keyUp(id self, SEL _cmd, id event) {
    KeyCodeFunc f = reinterpret_cast<KeyCodeFunc>(objc_msgSend);
    hs.keyUpHandler(static_cast<int>(f(event, sel_registerName("keyCode"))), modifiersFromEvent(event));
}

extern "C" void mouseDown(id self, SEL _cmd, id event) {
    LocationFunc f = reinterpret_cast<LocationFunc>(objc_msgSend);
    auto point = f(event, sel_registerName("locationInWindow"));
    
    hs.mouseDownHandler(point.x, point.y, mouseButtonFromEvent(event), modifiersFromEvent(event));
}

extern "C" void mouseUp(id self, SEL _cmd, id event) {
    LocationFunc f = reinterpret_cast<LocationFunc>(objc_msgSend);
    auto point = f(event, sel_registerName("locationInWindow"));

    hs.mouseUpHandler(point.x, point.y, mouseButtonFromEvent(event), modifiersFromEvent(event));
}

extern "C" void mouseMoved(id self, SEL _cmd, id event) {
    LocationFunc f = reinterpret_cast<LocationFunc>(objc_msgSend);
    auto point = f(event, sel_registerName("locationInWindow"));

    hs.mouseMovedHandler(point.x, point.y, modifiersFromEvent(event));
}

extern "C" void scrollWheel(id self, SEL _cmd, id event) {
    ScrollWheelFunc f = reinterpret_cast<ScrollWheelFunc>(objc_msgSend);
    LocationFunc pointFunc = reinterpret_cast<LocationFunc>(objc_msgSend);
    auto deltaX = f(event, sel_registerName("scrollingDeltaX"));
    auto deltaY = f(event, sel_registerName("scrollingDeltaY"));
    auto point = pointFunc(event, sel_registerName("locationInWindow"));

    hs.scrollWheelHandler(deltaX, deltaY, point.x, point.y, modifiersFromEvent(event));
}

MTKViewDelegate::MTKViewDelegate(MTL::Device* device, MTK::View* view):
    view{view},
    renderer{new Renderer{device, view}},
    cv{},
    m{},
    ready{false}
{
    renderer->makeCurrent();
    resizeThread = std::thread(&MTKViewDelegate::resizeWatcher, this);
}

void MTKViewDelegate::registerInspector(Inspector::Inspector& inspector) {
    renderer->registerInspector(inspector);
}

void MTKViewDelegate::resizeWatcher() {
    while (true) {
        std::unique_lock<std::mutex> lock(m);

        cv.wait_for(lock, std::chrono::milliseconds(100), [&] { return ready; });
        if (!ready) continue;

        ready = false;
        lock.unlock();
        ContextManager::getContext().updateView();
    }

}

void MTKViewDelegate::drawableSizeWillChange(MTK::View* view, CGSize frameSize) {
    std::lock_guard<std::mutex> lock(m);
    ready = true;
    cv.notify_one();
}


MTKViewDelegate::~MTKViewDelegate() {

    if (resizeThread.joinable()) {
        resizeThread.join();
    }
}


void MTKViewDelegate::drawInMTKView(MTK::View* view)
{
     renderer->draw();
}


AppDelegate::AppDelegate()
{
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

void AppDelegate::setFocused(tree::TreeNode* node) {
    if (focused == node) return;

    auto* oldFocused = focused;
    focused = node;

    if (oldFocused) {
        Event blur;
        blur.type = EventType::Blur;
        blur.payload = FocusPayload{};
        if (inspector) {
            inspector->observe(blur);
        }
        oldFocused->dispatch(blur);
    }

    if (focused) {
        Event focus;
        focus.type = EventType::Focus;
        focus.payload = FocusPayload{};
        if (inspector) {
            inspector->observe(focus);
        }
        focused->dispatch(focus);
    }
}

tree::TreeNode* AppDelegate::hitTest(float x, float y, simd_float2& testPoint) {
    auto frameInfo = viewDelegate->renderer->getFrameInfo();
    testPoint = toViewPoint(x, y, frameInfo.height);

    auto& currTree = viewDelegate->renderer->rootTree;
    auto* root = currTree.getRoot();
    if (!root) return nullptr;

    return currTree.hitTestRecursive(root, testPoint);
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
    
    window->setContentView(view);

    runtime::ContextManager::initContext(device, view);

    viewDelegate = std::make_unique<MTKViewDelegate>(device,view);

    if constexpr (GUI_INSPECTOR_ENABLED) {
        inspector.emplace();
        viewDelegate->registerInspector(*inspector);
    }

    view->setDelegate(viewDelegate.get());
    
    // adding input!!!
    id objcInstance = reinterpret_cast<id>(view);
    Class cls = object_getClass(objcInstance);

    
    hs.keyDownHandler = [this](int keyCode, Modifiers modifiers){
        Event e;
        e.type = EventType::KeyDown;
        e.payload = KeyboardPayload{
            .keyCode = keyCode,
            .modifiers = modifiers
        };

        if (this->inspector) {
            this->inspector->observe(e);
        }

        if (this->focused) {
            this->focused->dispatch(e);
        }
    };

    hs.keyUpHandler = [this](int keyCode, Modifiers modifiers){
        Event e;
        e.type = EventType::KeyUp;
        e.payload = KeyboardPayload{
            .keyCode = keyCode,
            .modifiers = modifiers
        };

        if (this->inspector) {
            this->inspector->observe(e);
        }

        if (this->focused) {
            this->focused->dispatch(e);
        }
    };
    
    hs.mouseDownHandler = [this](float x, float y, MouseButton button, Modifiers modifiers){
        simd_float2 testPoint{0.0f, 0.0f};
        auto* htnode = this->hitTest(x, y, testPoint);
        if (!htnode) {
            this->mouseDownTarget = nullptr;
            return;
        }

        Event e;
        e.type = EventType::MouseDown;
        e.payload = MousePayload{
            .position = testPoint,
            .button = button,
            .modifiers = modifiers
        };

        if (this->inspector) {
            this->inspector->observe(e);
        }
        
        this->mouseDownTarget = htnode;
        this->setFocused(htnode);
        htnode->dispatch(e);
    };

    hs.mouseUpHandler = [this](float x, float y, MouseButton button, Modifiers modifiers){
        simd_float2 testPoint{0.0f, 0.0f};
        auto* htnode = this->hitTest(x, y, testPoint);
        if (!htnode) {
            this->mouseDownTarget = nullptr;
            return;
        }

        MousePayload payload{
            .position = testPoint,
            .button = button,
            .modifiers = modifiers
        };

        Event e;
        e.type = EventType::MouseUp;
        e.payload = payload;

        if (this->inspector) {
            this->inspector->observe(e);
        }

        htnode->dispatch(e);

        if (this->mouseDownTarget == htnode) {
            Event click;
            click.type = EventType::Click;
            click.payload = payload;
            if (this->inspector) {
                this->inspector->observe(click);
            }
            htnode->dispatch(click);
        }

        this->mouseDownTarget = nullptr;
    };

    hs.mouseMovedHandler = [this](float x, float y, Modifiers modifiers){
        simd_float2 testPoint{0.0f, 0.0f};
        auto* htnode = this->hitTest(x, y, testPoint);

        MousePayload payload{
            .position = testPoint,
            .button = MouseButton::None,
            .modifiers = modifiers
        };

        if (this->hovered != htnode) {
            if (this->hovered) {
                Event leave;
                leave.type = EventType::MouseLeave;
                leave.payload = payload;
                this->hovered->dispatch(leave);
            }

            this->hovered = htnode;

            if (this->hovered) {
                Event enter;
                enter.type = EventType::MouseEnter;
                enter.payload = payload;
                this->hovered->dispatch(enter);
            }
        }

        if (this->hovered) {
            Event move;
            move.type = EventType::MouseMove;
            move.payload = payload;
            if (this->inspector) {
                this->inspector->observe(move);
            }
            this->hovered->dispatch(move);
        }
    };

    hs.scrollWheelHandler = [this](float dx, float dy, float x, float y, Modifiers modifiers) {
        simd_float2 testPoint{0.0f, 0.0f};
        auto* scrollNode = this->hitTest(x, y, testPoint);

        auto& currTree = this->viewDelegate->renderer->rootTree;
        auto* root = currTree.getRoot();
        if (!root) return;

        if (!scrollNode) {
            scrollNode = root;
        }

        Event e;
        e.type = EventType::ScrollWheel;
        e.payload = ScrollPayload{
            .position = testPoint,
            .dx = dx,
            .dy = dy,
            .modifiers = modifiers
        };

        if (this->inspector) {
            this->inspector->observe(e);
        }

        if (auto* dirtyScrollNode = scrollNode->dispatch(e)) {
            currTree.markDirty(dirtyScrollNode,
                DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize);
        }
    };

    class_addMethod(cls, sel_registerName("acceptsFirstResponder"),
                    reinterpret_cast<IMP>(acceptsFirstResponder), "B@:");
    class_addMethod( cls , sel_registerName("keyDown:"), reinterpret_cast<IMP>(keyDown), "v@:@");
    class_addMethod( cls , sel_registerName("keyUp:"), reinterpret_cast<IMP>(keyUp), "v@:@");
    class_addMethod( cls , sel_registerName("mouseDown:"), reinterpret_cast<IMP>(mouseDown), "v@:@");
    class_addMethod( cls , sel_registerName("rightMouseDown:"), reinterpret_cast<IMP>(mouseDown), "v@:@");
    class_addMethod( cls , sel_registerName("otherMouseDown:"), reinterpret_cast<IMP>(mouseDown), "v@:@");
    class_addMethod( cls , sel_registerName("mouseUp:"), reinterpret_cast<IMP>(mouseUp), "v@:@");
    class_addMethod( cls , sel_registerName("rightMouseUp:"), reinterpret_cast<IMP>(mouseUp), "v@:@");
    class_addMethod( cls , sel_registerName("otherMouseUp:"), reinterpret_cast<IMP>(mouseUp), "v@:@");
    class_addMethod( cls , sel_registerName("mouseMoved:"), reinterpret_cast<IMP>(mouseMoved), "v@:@");
    class_addMethod( cls , sel_registerName("scrollWheel:"), reinterpret_cast<IMP>(scrollWheel), "v@:@");

    using SetBoolFunc = void(*)(id, SEL, bool);
    auto setAcceptsMouseMovedEvents = reinterpret_cast<SetBoolFunc>(objc_msgSend);
    setAcceptsMouseMovedEvents(reinterpret_cast<id>(window), sel_registerName("setAcceptsMouseMovedEvents:"), true);


    
    
    // run some tests (no visual updates)
    // UIContext ctx {this->device, this->view};
    // index(ctx);
    
    
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
