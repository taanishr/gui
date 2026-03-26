#include "index.hpp"
#include "new_arch.hpp"
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

    };



    // div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .position(NewArch::Position::Relative)
    // (
    //     div()
    //         .width(NewArch::Size::percent(1.0))
    //         // .height(NewArch::Size::px(100.0))
    //         .color(simd_float4{0.9,0.9,0.9,1.0})
    //         .display(NewArch::Display::Flex)
    //         .flexGap(NewArch::Size::percent(0.01))
    //         // .alignItems(NewArch::AlignItems::Center)
    //         .flexDirection(NewArch::FlexDirection::Col)
    //     (
    //         div(NewArch::Size::px(90), NewArch::Size::px(80), simd_float4{1.0,0.3,0.3,1.0}),
    //         div()
    //         // .width(NewArch::Size::px(80))
    //         .color(simd_float4{0.016,0.875,0.449,1.0})
    //         (
    //             text("hello world fsdfsdfsdfsdfsdfsdf").fontSize(NewArch::Size::pt(16.0)).color(simd_float4{0.0,0.0,0.0,1.0})
    //         )
    //         ,div(NewArch::Size::px(90), NewArch::Size::px(90), simd_float4{0.3,0.3,1.0,1.0})
    //     )
    // );

div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
(
    div(NewArch::Size::px(200), NewArch::Size::px(300), simd_float4{0.9,0.9,0.9,1.0})
        .display(NewArch::Display::Flex)
        .flexDirection(NewArch::FlexDirection::Col)
    (
        div(NewArch::Size::px(100), NewArch::Size::px(200), simd_float4{1.0,0.3,0.3,1.0})
            .flexShrink(NewArch::Size::px(0)),
        div(NewArch::Size::px(100), NewArch::Size::px(200), simd_float4{0.3,1.0,0.3,1.0})
    )
);

    //     div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .position(NewArch::Relative) // no containing block anymore; we shall resort to relativity here
    //     .left(NewArch::Size::px(10))
    // (
    //      div(NewArch::Size::percent(0.2), NewArch::Size::percent(1.0), simd_float4{1.0,0.5,1.0,0.8})
    //     // .paddingTop(NewArch::Size::px(0))
    //      (
    //         div()
    //             // .position(NewArch::Absolute)
    //             .width(NewArch::Size::px(70))
    //             // .height(NewArch::Size::px(30))
    //             .color(simd_float4{0.498,0.0,1.0,1.0})
    //             .marginTop(30)
    //             .marginLeft(NewArch::Size::autoSize())
    //             .marginRight(NewArch::Size::autoSize())
    //             .cornerRadius(NewArch::Size::percent(0.075))
    //             .paddingLeft(NewArch::Size::px(9.0))
    //             .paddingTop(NewArch::Size::px(10.0))
    //             .borderColor(simd_float4{0.77,0.71,1.0,1.0})
    //             .borderWidth(NewArch::Size::px(1.0))
    //             .addEventListener(EventType::MouseDown, onClick)
    //         (
    //             text("Startfsd"),
    //             text("fsdfsdfsda dads sdsfsdsds")
    //         )
    //         // ,div(
    //         //     NewArch::Size::px(60), NewArch::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
    //         // ).marginTop(10)
    //      )
    // );
}

