#include "index.hpp"
#include "events.hpp"
#include <print>


auto index() -> void {
    div(200, 200, simd_float4{0,1,0,1}).position(NewArch::Position::Absolute).left(100).top(200)
    .paddingLeft(20.0).addEventListener(EventType::MouseDown, [](auto& desc, const Event& event){ 
            std::println("hello world!!");
            desc.color = simd_float4{0,0.5,0,0.5};
        })
    (
        div(50, 50, simd_float4{1,1,1,1}),
        text("hello \nworld", 64.0).color(simd_float4{0.0,0.0,1.0,1.0}).addEventListener(EventType::MouseDown, [](auto& desc, const Event& event){ 
            std::println("hello world!!");
            desc.text = "omg";
        }).addEventListener(EventType::KeyDown, [](auto& desc, const Event& event){
            char ch = event.get<EventType::KeyDown>().keyCode;
            std::println("{}", ch);
            desc.text = ch;

        })
    );
}