#include "index.hpp"
#include "context_manager.hpp"
#include "events.hpp"
#include "new_arch.hpp"
#include <print>
#include <simd/vector_types.h>

    // div(200, 200, simd_float4{0,1,0,1}).position(NewArch::Position::Absolute).left(100).top(200)
    // .paddingLeft(20.0).addEventListener(EventType::MouseDown, [](auto& desc, const Event& event){ 
    //         std::println("hello world!!");
    //         desc.color = simd_float4{0,0.5,0,0.5};
    //     })
    // (
    //     div(50, 50, simd_float4{1,1,1,1}),
    //     text("hello \nworld", 64.0).color(simd_float4{0.0,0.0,1.0,1.0}).addEventListener(EventType::MouseDown, [](auto& desc, const Event& event){ 
    //         std::println("hello world!!");
    //         desc.text = "omg";
    //     }).addEventListener(EventType::KeyDown, [](auto& desc, const Event& event){
    //         char ch = event.get<EventType::KeyDown>().keyCode;
    //         std::println("{}", ch);
    //         desc.text = ch;

    //     })
    // );

const std::string ArialBold = "/System/Library/Fonts/Supplemental/Arial Bold.ttf";

auto index() -> void {

    auto frameHeight = (float)ContextManager::getContext().view->drawableSize().height;
    auto frameWidth = (float)ContextManager::getContext().view->drawableSize().width;

    auto onClick = [](auto& desc, const Event& event){
        std::println("hello world");
    };

    div(frameHeight, frameWidth, simd_float4{1.0,1.0,1.0,1.0})(
         div(60, 30, simd_float4{0.498,0.0,1.0,1.0})
            .position(NewArch::Absolute).left(20)
            .top(30)
            .cornerRadius(7.5)
            .paddingLeft(9.5)
            .paddingTop(4.5)
            .borderColor(simd_float4{0.77,0.71,1.0,1.0})
            .borderWidth(1.0)
            .addEventListener(EventType::MouseDown, onClick)
        (
            text("Start")
                .fontSize(18.0)
                .font(ArialBold)
        )
    );
}

