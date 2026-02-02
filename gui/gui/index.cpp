#include "index.hpp"
#include "sizing.hpp"
#include <print>
#include <simd/vector_types.h>

const std::string Arial = "/System/Library/Fonts/Supplemental/Arial.ttf";
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
        .borderWidth(NewArch::Size::px(1.0))
        .position(NewArch::Absolute)
        .left(NewArch::Size::px(10))
    (
         div(NewArch::Size::percent(0.2), NewArch::Size::percent(1.0), simd_float4{1.0,0.5,1.0,0.8})(
            div(NewArch::Size::px(60), NewArch::Size::px(30), simd_float4{0.498,0.0,1.0,1.0})
                .marginTop(30)
                .marginLeft(NewArch::Size::autoSize())
                .marginRight(NewArch::Size::autoSize())
                .cornerRadius(NewArch::Size::percent(7.5))
                .paddingLeft(NewArch::Size::px(9.0))
                .paddingTop(NewArch::Size::px(4.5))
                .borderColor(simd_float4{0.77,0.71,1.0,1.0})
                .borderWidth(NewArch::Size::px(1.0))
                .addEventListener(EventType::MouseDown, onClick)
            (
                text("Startfsd")
                    .fontSize(NewArch::Size::pt(48.0))
                    .font(Arial)
                    .marginLeft(10)
                    .marginRight(10)
                    .color(simd_float4{0,0,0,1})
                    ,
                text("fsdfsdfsda   dads sdsfsdsds")
                    .fontSize(NewArch::Size::pt(48.0))
                    .font(Arial)
                    .color(simd_float4{0,0,0,1})
            ),
            div(
                NewArch::Size::px(60), NewArch::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
            ).marginTop(10)
         )
    );
}

