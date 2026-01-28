#include "index.hpp"
#include <print>
#include <simd/vector_types.h>

const std::string ArialBold = "/System/Library/Fonts/Supplemental/Arial Bold.ttf";

static int count = 0;

auto index() -> void {

    auto onClick = [](auto& desc, const Event& event){
        count += 1;

        if (count % 2 == 0) {
            desc.color = simd_float4{1.0,0.0,0.0,1.0};
        }else {
            desc.color = simd_float4{0.0,0.0,1.0,1.0};
        }

        std::println("hello world {}", count);
    };

    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
        .borderColor(simd_float4{0.77,0.71,1.0,1.0})
        .borderWidth(1.0)
    (
         div(NewArch::Size::percent(0.2), NewArch::Size::percent(1.0), simd_float4{1.0,0.5,1.0,0.8})(
            div(NewArch::Size::px(60), NewArch::Size::px(30), simd_float4{0.498,0.0,1.0,1.0})
                // .position(NewArch::Absolute)
                // .top(30)
                .marginTop(30)
                .marginBottom(20)
                .marginLeft(20)
                .cornerRadius(7.5)
                .paddingLeft(9.0)
                .paddingTop(4.5)
                .borderColor(simd_float4{0.77,0.71,1.0,1.0})
                .borderWidth(1.0)
                .addEventListener(EventType::MouseDown, onClick)
            (
                text("Startfsd")
                    .fontSize(16.0)
                    .font(ArialBold)
                    .marginLeft(10)
                    .marginRight(10),

                    text("fsdfsdf")
                    .fontSize(16.0)
                    .font(ArialBold)
            ),
            div(
                NewArch::Size::px(60), NewArch::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
            ).marginTop(10)
         )
    );
}

