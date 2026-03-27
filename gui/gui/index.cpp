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

    // // images cause like a 60mb increase in memory usage lol; need to investigate
    //     // turns out they just were not being downsampled

    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
        .display(NewArch::Display::Flex)
        .alignItems(NewArch::AlignItems::Center)
        .justifyContent(NewArch::JustifyContent::Center)
    (
        div()
            .color(simd_float4{1.0,1.0,1.0,1.0})
            .cornerRadius(NewArch::Size::px(16))
            .paddingLeft(NewArch::Size::px(12))
            .paddingRight(NewArch::Size::px(12))
            .display(NewArch::Display::Flex)
            .flexDirection(NewArch::FlexDirection::Col)
            .alignItems(NewArch::AlignItems::Center) // expands maxWidth for... some reason?
            .flexGap(NewArch::Size::px(12))
        (
            image("/Users/treja/Downloads/sf90.jpg", NewArch::Size::px(80), NewArch::Size::px(80))
                .cornerRadius(NewArch::Size::percent(0.5))
            ,text("Sarah Johnson")
                .fontSize(NewArch::Size::pt(18))
                .color(simd_float4{0.1,0.1,0.1,1.0})
            ,text("Product Designer @ Figma")
                .fontSize(NewArch::Size::pt(13))
                .color(simd_float4{0.5,0.5,0.5,1.0})
            ,div(NewArch::Size::percent(1.0), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
                .display(NewArch::Display::Flex)
                .justifyContent(NewArch::JustifyContent::SpaceAround)
            (
                div(NewArch::Size::px(70), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text("284").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0})
                    ,text("Posts").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
                ),
                div(NewArch::Size::px(70), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text("12.4k").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text("Followers").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
                ),
                div(NewArch::Size::px(70), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text("891").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text("Following").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
                )
            ),
            div(NewArch::Size::px(120), NewArch::Size::px(40), simd_float4{0.4,0.3,1.0,1.0})
                .cornerRadius(NewArch::Size::px(20))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::Center)
                .addEventListener(EventType::MouseDown, onClick)
            (
                text("Follow").fontSize(NewArch::Size::pt(14)).color(simd_float4{1.0,1.0,1.0,1.0})
            )
        )
    );

    // div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .position(NewArch::Position::Relative)
    // (
    //     div()
    //         .width(NewArch::Size::percent(1.0))
    //         .height(NewArch::Size::px(500.0))
    //         .color(simd_float4{0.8,0.8,0.8,1.0})
    //         .display(NewArch::Display::Flex)
    //         .flexGap(NewArch::Size::px(0.01))
    //         // .alignItems(NewArch::AlignItems::Center)
    //         .justifyContent(JustifyContent::FlexEnd)
    //         .flexDirection(NewArch::FlexDirection::ColReverse)
    //     (
    //         div(NewArch::Size::px(90), NewArch::Size::px(80), simd_float4{1.0,0.3,0.3,1.0}),
    //         div()
    //         // .width(NewArch::Size::px(80))
    //         // .position(Position::Absolute)
    //         .padding(NewArch::Size::px(12))
    //         .color(simd_float4{0.016,0.875,0.449,1.0})
    //         (
    //             text("hello world fsdfsdfsdfsdfsdfsdf").fontSize(NewArch::Size::pt(16.0)).color(simd_float4{0.0,0.0,0.0,1.0})
    //         )
    //         ,div(NewArch::Size::px(90), NewArch::Size::px(90), simd_float4{0.3,0.3,1.0,1.0})
    //     )
    // );


    //     div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .position(NewArch::Position::Relative) // no containing block anymore; we shall resort to relativity here
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
    //         ,div(
    //             NewArch::Size::px(60), NewArch::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
    //         ).marginTop(10)
    //      )
    // );
}

