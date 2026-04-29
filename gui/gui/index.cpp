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

    // images cause like a 60mb increase in memory usage lol; need to investigate
    // turns out they just were not being downsampled

    // // seg faults fucking hell
    // using S = NewArch::Size;

    // // Grid demo: holy grail layout
    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
    //     .display(NewArch::Display::Grid)
    //     .gridTemplateColumns({S::fr(1), S::fr(2), S::fr(1)})
    //     .gridTemplateRows({S::px(60), S::fr(1), S::px(40)})
    //     .gridColumnGap(S::px(8))
    //     .gridRowGap(S::px(8))
    //     .padding(S::px(8))
    // (
    //     // Header — spans all 3 columns
    //     div().gridColumn(1, 4).gridRow(1, 2)
    //         .color(simd_float4{0.2,0.4,0.8,1.0})
    //         .cornerRadius(S::px(8))
    //         .display(NewArch::Display::Flex)
    //         .alignItems(NewArch::AlignItems::Center)
    //         .justifyContent(NewArch::JustifyContent::Center)
    //     (
    //         text(U"Header").fontSize(S::pt(20)).color(simd_float4{1,1,1,1})
    //     ),

    //     // Left sidebar
    //     div().gridColumn(1, 2).gridRow(2, 3)
    //         .color(simd_float4{0.9,0.9,0.95,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text(U"Sidebar").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
    //     ),

    //     // Main content
    //     div().gridColumn(2, 3).gridRow(2, 3)
    //         .color(simd_float4{1.0,1.0,1.0,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text(U"Main Content").fontSize(S::pt(14)).color(simd_float4{0.1,0.1,0.1,1})
    //     ),

    //     // Right sidebar
    //     div().gridColumn(3, 4).gridRow(2, 3)
    //         .color(simd_float4{0.9,0.9,0.95,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text(U"Panel").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
    //     ),

    //     // Footer — spans all 3 columns
    //     div().gridColumn(1, 4).gridRow(3, 4)
    //         .color(simd_float4{0.3,0.3,0.35,1.0})
    //         .cornerRadius(S::px(8))
    //         .display(NewArch::Display::Flex)
    //         .alignItems(NewArch::AlignItems::Center)
    //         .justifyContent(NewArch::JustifyContent::Center)
    //     (
    //         text(U"Footer").fontSize(S::pt(14)).color(simd_float4{1,1,1,1})
    //     )
    // );

   



    
    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    .display(NewArch::Display::Flex)
    .paddingTop(NewArch::Size::px(32.0))
    .flexDirection(NewArch::FlexDirection::Col)
(
    // Header
    div(NewArch::Size::percent(1.0), NewArch::Size::px(56), simd_float4{1.0,1.0,1.0,1.0})
        .display(NewArch::Display::Flex)
        .alignItems(NewArch::AlignItems::Center)
        .justifyContent(NewArch::JustifyContent::SpaceBetween)
        .paddingLeft(NewArch::Size::px(16))
        .paddingRight(NewArch::Size::px(16))
        .borderColor(simd_float4{0.88,0.88,0.88,1.0})
        .borderWidth(NewArch::Size::px(1.0))
        .flexShrink(NewArch::Size::px(0.0))
    (
        // Left
        div()
            .color(simd_float4{0.0,0.0,0.0,0.0})
            .display(NewArch::Display::Flex)
            .alignItems(NewArch::AlignItems::Center)
            .flexGap(NewArch::Size::px(8))
        (
            div(NewArch::Size::px(40), NewArch::Size::px(40), simd_float4{0.96,0.96,0.96,1.0})
                .cornerRadius(NewArch::Size::px(20))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::Center)
            (
                text(U"=").fontSize(NewArch::Size::pt(18)).color(simd_float4{0.5,0.5,0.5,1.0})
            ),
            div()
                .color(simd_float4{0.96,0.96,0.96,1.0})
                .paddingLeft(NewArch::Size::px(12))
                .paddingRight(NewArch::Size::px(12))
                .height(NewArch::Size::px(32))
                .cornerRadius(NewArch::Size::px(6))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
            (
                text(U"You: San Francisco").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
            )
        ),
        // Right
        div()
            .color(simd_float4{0.0,0.0,0.0,0.0})
            .display(NewArch::Display::Flex)
            .alignItems(NewArch::AlignItems::Center)
            .flexGap(NewArch::Size::px(8))
        (
            div()
                .color(simd_float4{0.96,0.96,0.96,1.0})
                .height(NewArch::Size::px(36))
                .paddingLeft(NewArch::Size::px(16))
                .paddingRight(NewArch::Size::px(16))
                .cornerRadius(NewArch::Size::px(6))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
            (
                text(U"Import").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
            ),
            div()
                .color(simd_float4{0.1,0.1,0.1,1.0})
                .height(NewArch::Size::px(36))
                .paddingLeft(NewArch::Size::px(16))
                .paddingRight(NewArch::Size::px(16))
                .cornerRadius(NewArch::Size::px(6))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
            (
                text(U"Add trip").fontSize(NewArch::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
            )
        )
    ),
    // Body
    div()
        .color(simd_float4{0.0,0.0,0.0,0.0})
        .display(NewArch::Display::Flex)
        .flexGrow(NewArch::Size::px(1))
    (
        // Sidebar
        div()
            // .width(NewArch::Size::px(160.0))
            .color(simd_float4{1.0,1.0,1.0,1.0})
            .borderColor(simd_float4{0.88,0.88,0.88,1.0})
            .borderWidth(NewArch::Size::px(1.0))
            .display(NewArch::Display::Flex)
            .flexDirection(NewArch::FlexDirection::Col)
            .flexGap(NewArch::Size::px(4))
            .padding(NewArch::Size::px(12))
            .flexShrink(NewArch::Size::px(0.0))
        (
            text(U"TRIPS").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
            div()
                .width(NewArch::Size::percent(1.0))
                .color(simd_float4{0.96,0.96,0.96,1.0})
                .cornerRadius(NewArch::Size::px(8))
                .paddingLeft(NewArch::Size::px(12))
                .paddingRight(NewArch::Size::px(12))
                .paddingTop(NewArch::Size::px(8))
                .paddingBottom(NewArch::Size::px(8))
                .display(NewArch::Display::Flex)
                .flexDirection(NewArch::FlexDirection::Col)
                .flexGap(NewArch::Size::px(2))
            (
                text(U"Tokyo & Kyoto").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text(U"Mar 10 - Mar 24").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
            ),
            div()
                .width(NewArch::Size::percent(1.0))
                .color(simd_float4{1.0,1.0,1.0,1.0})
                .cornerRadius(NewArch::Size::px(8))
                .paddingLeft(NewArch::Size::px(12))
                .paddingRight(NewArch::Size::px(12))
                .paddingTop(NewArch::Size::px(8))
                .paddingBottom(NewArch::Size::px(8))
                .display(NewArch::Display::Flex)
                .flexDirection(NewArch::FlexDirection::Col)
                .flexGap(NewArch::Size::px(2))
            (
                text(U"NYC Weekend").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text(U"Apr 4 - Apr 7").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
            ),
            div()
                .width(NewArch::Size::percent(1.0))
                .color(simd_float4{1.0,1.0,1.0,1.0})
                .cornerRadius(NewArch::Size::px(8))
                .paddingLeft(NewArch::Size::px(12))
                .paddingRight(NewArch::Size::px(12))
                .paddingTop(NewArch::Size::px(8))
                .paddingBottom(NewArch::Size::px(8))
                .display(NewArch::Display::Flex)
                .flexDirection(NewArch::FlexDirection::Col)
                .flexGap(NewArch::Size::px(2))
            (
                text(U"London + Paris").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text(U"Jun 1 - Jun 12").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
            )
        ),
        // Main content
        div()
            .color(simd_float4{0.97,0.97,0.97,1.0})
            .flexGrow(NewArch::Size::px(1))
            .display(NewArch::Display::Flex)
            .flexDirection(NewArch::FlexDirection::Col)
            .padding(NewArch::Size::px(24))
            .flexGap(NewArch::Size::px(12))
        (
            // Title row
            div()
                .color(simd_float4{0.0,0.0,0.0,0.0})
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::SpaceBetween)
            (
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .flexGap(NewArch::Size::px(2))
                (
                    text(U"Tokyo & Kyoto").fontSize(NewArch::Size::pt(20)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text(U"Mar 10 - Mar 24 · 14 days").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
                ),
                div()
                    .color(simd_float4{0.96,0.96,0.96,1.0})
                    .paddingLeft(NewArch::Size::px(16))
                    .paddingRight(NewArch::Size::px(16))
                    .height(NewArch::Size::px(32))
                    .cornerRadius(NewArch::Size::px(6))
                    .display(NewArch::Display::Flex)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text(U"Edit").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
                )
            ),
            // Flight
            div(NewArch::Size::percent(1.0), NewArch::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
                .cornerRadius(NewArch::Size::px(12))
                .paddingLeft(NewArch::Size::px(16))
                .paddingRight(NewArch::Size::px(16))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::SpaceBetween)
            (
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .flexGap(NewArch::Size::px(4))
                (
                    div()
                        .color(simd_float4{0.0,0.0,0.0,0.0})
                        .display(NewArch::Display::Flex)
                        .flexGap(NewArch::Size::px(4))
                        .alignItems(NewArch::AlignItems::Center)
                    (
                        svg("/Users/treja/projects/gui/gui/sources/plane.svg")
                            .width(NewArch::Size::px(22))
                            .height(NewArch::Size::px(22)),
                        text(U"SFO -> NRT").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                    )
                    ,text(U"Mar 10 · United 837 · 11h 30m").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
                ),
                div()
                    .color(simd_float4{0.93,0.88,1.0,1.0})
                    .paddingLeft(NewArch::Size::px(12))
                    .paddingRight(NewArch::Size::px(12))
                    .height(NewArch::Size::px(24))
                    .cornerRadius(NewArch::Size::px(12))
                    .display(NewArch::Display::Flex)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text(U"[Flight]").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.4,0.3,1.0,1.0})
                )
            ),
            // Hotel
            div(NewArch::Size::percent(1.0), NewArch::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
                .cornerRadius(NewArch::Size::px(12))
                .paddingLeft(NewArch::Size::px(16))
                .paddingRight(NewArch::Size::px(16))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::SpaceBetween)
            (
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .flexGap(NewArch::Size::px(4))
                (
                    text(U"Park Hyatt Tokyo").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text(U"Mar 11 - Mar 17 · 6 nights").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
                ),
                div()
                    .color(simd_float4{0.85,0.93,1.0,1.0})
                    .paddingLeft(NewArch::Size::px(12))
                    .paddingRight(NewArch::Size::px(12))
                    .height(NewArch::Size::px(24))
                    .cornerRadius(NewArch::Size::px(12))
                    .display(NewArch::Display::Flex)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text(U"[Hotel]").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.2,0.6,0.9,1.0})
                )
            ),
            // Train
            div(NewArch::Size::percent(1.0), NewArch::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
                .cornerRadius(NewArch::Size::px(12))
                .paddingLeft(NewArch::Size::px(16))
                .paddingRight(NewArch::Size::px(16))
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::SpaceBetween)
            (
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .flexDirection(NewArch::FlexDirection::Col)
                    .flexGap(NewArch::Size::px(4))
                (
                    text(U"Tokyo -> Kyoto").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text(U"Mar 17 · Shinkansen · 2h 15m").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
                ),
                div()
                    .color(simd_float4{0.85,1.0,0.9,1.0})
                    .paddingLeft(NewArch::Size::px(12))
                    .paddingRight(NewArch::Size::px(12))
                    .height(NewArch::Size::px(24))
                    .cornerRadius(NewArch::Size::px(12))
                    .display(NewArch::Display::Flex)
                    .alignItems(NewArch::AlignItems::Center)
                (
                    text(U"[Train]").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.1,0.7,0.4,1.0})
                )
            )
        )
        // Detail panel
        ,div()
            // .width(NewArch::Size::px(200.0))
            .color(simd_float4{1.0,1.0,1.0,1.0})
            .borderColor(simd_float4{0.88,0.88,0.88,1.0})
            .borderWidth(NewArch::Size::px(1.0))
            .display(NewArch::Display::Flex)
            .flexDirection(NewArch::FlexDirection::Col)
            .flexGap(NewArch::Size::px(16))
            .padding(NewArch::Size::px(16))
            .flexShrink(NewArch::Size::px(0.0))
        (
            div()
                .color(simd_float4{0.0,0.0,0.0,0.0})
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::SpaceBetween)
            (
                text(U"Flight Details").fontSize(NewArch::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text(U"x").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0})
            ),
            div()
                .color(simd_float4{0.0,0.0,0.0,0.0})
                .display(NewArch::Display::Flex)
                .flexDirection(NewArch::FlexDirection::Col)
                .flexGap(NewArch::Size::px(12))
            (
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text(U"Flight").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text(U"UA 837").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text(U"Departs").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text(U"10:45 AM").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text(U"Arrives").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text(U"3:15 PM +1").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text(U"Duration").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text(U"11h 30m").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text(U"Seat").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text(U"42A").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text(U"Class").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text(U"Economy").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                )
            ),
            div()
                .width(NewArch::Size::percent(1.0))
                .color(simd_float4{0.97,0.97,0.97,1.0})
                .cornerRadius(NewArch::Size::px(8))
                .padding(NewArch::Size::px(12))
                .display(NewArch::Display::Flex)
                .flexDirection(NewArch::FlexDirection::Col)
                .flexGap(NewArch::Size::px(4))
            (
                text(U"Local time at destination").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
                text(U"3:15 PM JST (UTC+9)").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
            )
        )
    )
);

//    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
//         .display(NewArch::Display::Flex)
//         .alignItems(NewArch::AlignItems::Center)
//         .justifyContent(NewArch::JustifyContent::Center)
//     (
//         div()
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(NewArch::Size::px(16))
//             .padding(NewArch::Size::px(12))
//             .display(NewArch::Display::Flex)
//             .flexDirection(NewArch::FlexDirection::Col)
//             .alignItems(NewArch::AlignItems::Center) // expands maxWidth for... some reason?
//             .flexGap(NewArch::Size::px(12))
//         (
//             image("/Users/treja/Downloads/sf90.jpg", NewArch::Size::px(80), NewArch::Size::px(80))
//                 .cornerRadius(NewArch::Size::percent(0.5))
//             ,text(U"Sarah Johnson")
//                 .fontSize(NewArch::Size::pt(18))
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//             ,text(U"Product Designer @ Figma")
//                 .fontSize(NewArch::Size::pt(13))
//                 .color(simd_float4{0.5,0.5,0.5,1.0})
//             ,//
//             div()
//                 .width(NewArch::Size::percent(1.0)) // this is broken now... percent sizing leads to this being full width of grandparent when html doesnt do that
//                 .height(NewArch::Size::px(50))
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(NewArch::Display::Flex)
//                 .justifyContent(NewArch::JustifyContent::SpaceAround)
//             (
//                 div(NewArch::Size::px(70), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(NewArch::Display::Flex)
//                     .flexDirection(NewArch::FlexDirection::Col)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"284").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     ,text(U"Posts").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//                 ,div(NewArch::Size::px(70), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(NewArch::Display::Flex)
//                     .flexDirection(NewArch::FlexDirection::Col)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"12.4k").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Followers").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div(NewArch::Size::px(70), NewArch::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(NewArch::Display::Flex)
//                     .flexDirection(NewArch::FlexDirection::Col)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"891").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Following").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//             )
//             ,
//             div(NewArch::Size::px(120), NewArch::Size::px(40), simd_float4{0.4,0.3,1.0,1.0})
//                 .cornerRadius(NewArch::Size::px(20))
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//                 .justifyContent(NewArch::JustifyContent::Center)
//                 .addEventListener(EventType::MouseDown, onClick)
//             (
//                 text(U"Follow").fontSize(NewArch::Size::pt(14)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     );



// // Dashboard layout: analytics overview
// div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{0.96,0.96,0.97,1.0})
//     .display(NewArch::Display::Grid)
//     .gridTemplateColumns({NewArch::Size::px(220), NewArch::Size::fr(1), NewArch::Size::fr(1), NewArch::Size::fr(1)})
//     .gridTemplateRows({NewArch::Size::px(56), NewArch::Size::px(120), NewArch::Size::fr(1), NewArch::Size::px(44)})
//     .gridColumnGap(NewArch::Size::px(10))
//     .gridRowGap(NewArch::Size::px(10))
//     .padding(NewArch::Size::px(10))
//     .paddingTop(NewArch::Size::px(32))
// (
//     // ── Topbar: columns 1-5, row 1 ──
//     div().gridColumn(1, 5).gridRow(1, 2)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .display(NewArch::Display::Flex)
//         .alignItems(NewArch::AlignItems::Center)
//         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//         .paddingLeft(NewArch::Size::px(20))
//         .paddingRight(NewArch::Size::px(20))
//     (
//         text(U"Analytics").fontSize(NewArch::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .height(NewArch::Size::px(32))
//                 .paddingLeft(NewArch::Size::px(14))
//                 .paddingRight(NewArch::Size::px(14))
//                 .cornerRadius(NewArch::Size::px(6))
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//             (
//                 text(U"Last 30 days").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//                 .height(NewArch::Size::px(32))
//                 .paddingLeft(NewArch::Size::px(14))
//                 .paddingRight(NewArch::Size::px(14))
//                 .cornerRadius(NewArch::Size::px(6))
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//             (
//                 text(U"Export").fontSize(NewArch::Size::pt(12)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     ),

//     // ── Stat card: Revenue ──
//     div().gridColumn(2, 3).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .padding(NewArch::Size::px(16))
//         .display(NewArch::Display::Flex)
//         .flexDirection(NewArch::FlexDirection::Col)
//         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//     (
//         text(U"Revenue").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"$48,230").fontSize(NewArch::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.85,1.0,0.9,1.0})
//                 .paddingLeft(NewArch::Size::px(8))
//                 .paddingRight(NewArch::Size::px(8))
//                 .height(NewArch::Size::px(20))
//                 .cornerRadius(NewArch::Size::px(10))
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//             (
//                 text(U"+12.4%").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.1,0.7,0.4,1.0})
//             )
//         ),
//         text(U"vs $42,900 last month").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Stat card: Users ──
//     div().gridColumn(3, 4).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .padding(NewArch::Size::px(16))
//         .display(NewArch::Display::Flex)
//         .flexDirection(NewArch::FlexDirection::Col)
//         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//     (
//         text(U"Active Users").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"8,412").fontSize(NewArch::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.93,0.88,1.0,1.0})
//                 .paddingLeft(NewArch::Size::px(8))
//                 .paddingRight(NewArch::Size::px(8))
//                 .height(NewArch::Size::px(20))
//                 .cornerRadius(NewArch::Size::px(10))
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//             (
//                 text(U"+3.1%").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.4,0.3,1.0,1.0})
//             )
//         ),
//         text(U"vs 8,160 last month").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Stat card: Churn ──
//     div().gridColumn(4, 5).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .padding(NewArch::Size::px(16))
//         .display(NewArch::Display::Flex)
//         .flexDirection(NewArch::FlexDirection::Col)
//         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//     (
//         text(U"Churn Rate").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"2.3%").fontSize(NewArch::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{1.0,0.9,0.88,1.0})
//                 .paddingLeft(NewArch::Size::px(8))
//                 .paddingRight(NewArch::Size::px(8))
//                 .height(NewArch::Size::px(20))
//                 .cornerRadius(NewArch::Size::px(10))
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//             (
//                 text(U"-0.4%").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.9,0.3,0.2,1.0})
//             )
//         ),
//         text(U"vs 2.7% last month").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Nav sidebar: column 1, rows 2-4 ──
//     div().gridColumn(1, 2).gridRow(2, 4)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .padding(NewArch::Size::px(12))
//         .display(NewArch::Display::Flex)
//         .flexDirection(NewArch::FlexDirection::Col)
//         .flexGap(NewArch::Size::px(4))
//     (
//         text(U"NAVIGATION").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .width(NewArch::Size::percent(1.0))
//             .color(simd_float4{0.1,0.1,0.1,1.0})
//             .cornerRadius(NewArch::Size::px(8))
//             .paddingLeft(NewArch::Size::px(12))
//             .paddingRight(NewArch::Size::px(12))
//             .paddingTop(NewArch::Size::px(8))
//             .paddingBottom(NewArch::Size::px(8))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"▪").fontSize(NewArch::Size::pt(10)).color(simd_float4{1.0,1.0,1.0,1.0}),
//             text(U"Overview").fontSize(NewArch::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
//         ),
//         div()
//             .width(NewArch::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(NewArch::Size::px(8))
//             .paddingLeft(NewArch::Size::px(12))
//             .paddingRight(NewArch::Size::px(12))
//             .paddingTop(NewArch::Size::px(8))
//             .paddingBottom(NewArch::Size::px(8))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"▪").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Revenue").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(NewArch::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(NewArch::Size::px(8))
//             .paddingLeft(NewArch::Size::px(12))
//             .paddingRight(NewArch::Size::px(12))
//             .paddingTop(NewArch::Size::px(8))
//             .paddingBottom(NewArch::Size::px(8))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"▪").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Users").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(NewArch::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(NewArch::Size::px(8))
//             .paddingLeft(NewArch::Size::px(12))
//             .paddingRight(NewArch::Size::px(12))
//             .paddingTop(NewArch::Size::px(8))
//             .paddingBottom(NewArch::Size::px(8))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"▪").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Reports").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(NewArch::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(NewArch::Size::px(8))
//             .paddingLeft(NewArch::Size::px(12))
//             .paddingRight(NewArch::Size::px(12))
//             .paddingTop(NewArch::Size::px(8))
//             .paddingBottom(NewArch::Size::px(8))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(8))
//         (
//             text(U"▪").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Settings").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         )
//     ),

//     // ── Chart area: columns 2-4, row 3 ──
//     div().gridColumn(2, 5).gridRow(3, 4)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .padding(NewArch::Size::px(16))
//         .display(NewArch::Display::Flex)
//         .flexDirection(NewArch::FlexDirection::Col)
//         .flexGap(NewArch::Size::px(12))
//     (
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .justifyContent(NewArch::JustifyContent::SpaceBetween)
//         (
//             text(U"Revenue over time").fontSize(NewArch::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(NewArch::Display::Flex)
//                 .flexGap(NewArch::Size::px(6))
//         (
//                 div()
//                     .color(simd_float4{0.96,0.96,0.96,1.0})
//                     .height(NewArch::Size::px(26))
//                     .paddingLeft(NewArch::Size::px(12))
//                     .paddingRight(NewArch::Size::px(12))
//                     .cornerRadius(NewArch::Size::px(6))
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Monthly").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.1,0.1,0.1,1.0})
//                     .height(NewArch::Size::px(26))
//                     .paddingLeft(NewArch::Size::px(12))
//                     .paddingRight(NewArch::Size::px(12))
//                     .cornerRadius(NewArch::Size::px(6))
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Weekly").fontSize(NewArch::Size::pt(11)).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .flexGrow(NewArch::Size::px(1))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::FlexEnd)
//             .flexGap(NewArch::Size::px(6))
//         (
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(0.61)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(NewArch::Size::px(4))(),
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(0.78)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(NewArch::Size::px(4))(),
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(0.50)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(NewArch::Size::px(4))(),
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(0.89)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(NewArch::Size::px(4))(),
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(0.72)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(NewArch::Size::px(4))(),
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(1.00)).color(simd_float4{0.4,0.3,1.0,1.0}).cornerRadius(NewArch::Size::px(4))(),
//             div().flexGrow(NewArch::Size::px(1)).height(NewArch::Size::percent(0.83)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(NewArch::Size::px(4))()
//         )
//     ),

//     // ── Footer: all columns, row 4 ──
//     div().gridColumn(1, 5).gridRow(4, 5)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(NewArch::Size::px(10))
//         .display(NewArch::Display::Flex)
//         .alignItems(NewArch::AlignItems::Center)
//         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//         .paddingLeft(NewArch::Size::px(20))
//         .paddingRight(NewArch::Size::px(20))
//     (
//         text(U"Last synced: Apr 17, 2026 at 9:41 AM").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .flexGap(NewArch::Size::px(16))
//         (
//             text(U"Privacy").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Terms").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Help").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//         )
//     )
// );

// using S = NewArch::Size;
// div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.96,0.98,1.0})
//         .display(NewArch::Display::Grid)
//         .gridTemplateColumns({S::px(250), S::fr(1), S::px(300)})
//         .gridTemplateRows({S::px(68), S::fr(1)})
//         .gridColumnGap(S::px(14))
//         .gridRowGap(S::px(14))
//         .paddingLeft(S::px(16))
//         .paddingRight(S::px(16))
//         .paddingBottom(S::px(16))
//         .paddingTop(S::px(28))
//     (
//         div().gridColumn(1, 4).gridRow(1, 2)
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(S::px(10))
//             .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//             .borderWidth(S::px(1))
//             .display(NewArch::Display::Flex)
//             .alignItems(NewArch::AlignItems::Center)
//             .justifyContent(NewArch::JustifyContent::SpaceBetween)
//             .paddingLeft(S::px(18))
//             .paddingRight(S::px(18))
//         (
//             div()
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//                 .flexGap(S::px(12))
//             (
//                 div(S::px(34), S::px(34), simd_float4{0.04,0.4,0.67,1.0})
//                     .cornerRadius(S::px(6))
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                     .justifyContent(NewArch::JustifyContent::Center)
//                 (
//                     text(U"in").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.94,0.97,1.0,1.0})
//                     .height(S::px(38))
//                     .paddingLeft(S::px(14))
//                     .paddingRight(S::px(64))
//                     .cornerRadius(S::px(19))
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Search").fontSize(S::pt(13)).color(simd_float4{0.41,0.49,0.56,1.0})
//                 )
//             ),
//             div()
//                 .display(NewArch::Display::Flex)
//                 .alignItems(NewArch::AlignItems::Center)
//                 .flexGap(S::px(10))
//             (
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Home").fontSize(S::pt(13)).color(simd_float4{0.12,0.16,0.2,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"My Network").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Jobs").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Messages").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(14)).paddingRight(S::px(14)).height(S::px(36)).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(18)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"Try Premium").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),

//         div().gridColumn(1, 2).gridRow(2, 3)
//             .display(NewArch::Display::Flex)
//             .flexDirection(NewArch::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .display(NewArch::Display::Flex)
//                 .flexDirection(NewArch::FlexDirection::Col)
//             (
//                 div()
//                     .height(S::px(74))
//                     .color(simd_float4{0.72,0.84,0.95,1.0})
//                     .cornerRadius(S::px(10))(),
//                 div()
//                     .paddingLeft(S::px(18))
//                     .paddingRight(S::px(18))
//                     .paddingBottom(S::px(18))
//                     .display(NewArch::Display::Flex)
//                     .flexDirection(NewArch::FlexDirection::Col)
//                     .alignItems(NewArch::AlignItems::Center)
//                     .flexGap(S::px(8))
//                 (
//                     div(S::px(84), S::px(84), simd_float4{0.1,0.45,0.77,1.0})
//                         .cornerRadius(S::px(42))
//                         .marginTop(S::px(-42))
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .justifyContent(NewArch::JustifyContent::Center)
//                     (
//                         text(U"TR").fontSize(S::pt(24)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     text(U"Taanish Reja").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{0.09,0.12,0.16,1.0}),
//                     text(U"Building a GPU-native UI kit").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                     div().height(S::px(1)).color(simd_float4{0.91,0.93,0.95,1.0})(),
//                     div()
//                         .display(NewArch::Display::Flex)
//                         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                     (
//                         text(U"Profile viewers").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text(U"128").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div()
//                         .display(NewArch::Display::Flex)
//                         .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                     (
//                         text(U"Post impressions").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text(U"2,481").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
//                     )
//                 )
//             ),
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(NewArch::Display::Flex)
//                 .flexDirection(NewArch::FlexDirection::Col)
//                 .flexGap(S::px(10))
//             (
//                 text(U"Recent").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 text(U"#metal").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text(U"#layoutengines").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text(U"#cpp").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text(U"#rendering").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0})
//             )
//         ),

//         div().gridColumn(2, 3).gridRow(2, 3)
//             .display(NewArch::Display::Flex)
//             .flexDirection(NewArch::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(NewArch::Display::Flex)
//                 .flexDirection(NewArch::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 div()
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                     .flexGap(S::px(12))
//                 (
//                     div(S::px(52), S::px(52), simd_float4{0.13,0.52,0.81,1.0})
//                         .cornerRadius(S::px(26))
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .justifyContent(NewArch::JustifyContent::Center)
//                     (
//                         text(U"TR").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     div()
//                         .color(simd_float4{0.96,0.97,0.98,1.0})
//                         .borderColor(simd_float4{0.84,0.87,0.9,1.0})
//                         .borderWidth(S::px(1))
//                         .cornerRadius(S::px(22))
//                         .height(S::px(44))
//                         .paddingLeft(S::px(18))
//                         .paddingRight(S::px(18))
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .flexGrow(S::px(1))
//                     (
//                         text(U"Start a post about the grid stress test").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                     )
//                 ),
//                 div()
//                     .display(NewArch::Display::Flex)
//                     .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                 (
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                     (
//                         text(U"Photo").fontSize(S::pt(12)).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                     (
//                         text(U"Video").fontSize(S::pt(12)).color(simd_float4{0.09,0.58,0.37,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center)
//                     (
//                         text(U"Write article").fontSize(S::pt(12)).color(simd_float4{0.79,0.47,0.08,1.0})
//                     )
//                 )
//             ),
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(NewArch::Display::Flex)
//                 .flexDirection(NewArch::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 div()
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                     .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .flexGap(S::px(12))
//                     (
//                         div(S::px(52), S::px(52), simd_float4{0.18,0.18,0.2,1.0})
//                             .cornerRadius(S::px(26))
//                             .display(NewArch::Display::Flex)
//                             .alignItems(NewArch::AlignItems::Center)
//                             .justifyContent(NewArch::JustifyContent::Center)
//                         (
//                             text(U"MS").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div()
//                             .display(NewArch::Display::Flex)
//                             .flexDirection(NewArch::FlexDirection::Col)
//                             .flexGap(S::px(3))
//                         (
//                             text(U"Maya Stone").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                             text(U"Design systems at Northstar").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                             text(U"2h").fontSize(S::pt(11)).color(simd_float4{0.58,0.62,0.67,1.0})
//                         )
//                     ),
//                     text(U"...").fontSize(S::pt(16)).color(simd_float4{0.53,0.57,0.62,1.0})
//                 ),
//                 text(U"Spent the morning rebuilding our composer with a new grid shell and nested flex rows. The ergonomics are getting close.")
//                     .fontSize(S::pt(14))
//                     .color(simd_float4{0.14,0.17,0.21,1.0}),
//                 div()
//                     .height(S::px(210))
//                     .color(simd_float4{0.9,0.94,0.98,1.0})
//                     .cornerRadius(S::px(12))
//                     .padding(S::px(18))
//                     .display(NewArch::Display::Grid)
//                     .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1), S::fr(1)})
//                     .gridTemplateRows({S::px(64), S::fr(1), S::px(46)})
//                     .gridColumnGap(S::px(10))
//                     .gridRowGap(S::px(10))
//                 (
//                     div().gridColumn(1, 5).gridRow(1, 2).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(10))
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text(U"Feed card inside a feed card").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     div().gridColumn(1, 2).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(2, 4).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(4, 5).gridRow(2, 3).color(simd_float4{0.84,0.91,0.98,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(1, 5).gridRow(3, 4).color(simd_float4{0.16,0.2,0.24,1.0}).cornerRadius(S::px(10))
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text(U"Nested grid preview").fontSize(S::pt(12)).color(simd_float4{0.92,0.94,0.97,1.0})
//                     )
//                 ),
//                 div()
//                     .display(NewArch::Display::Flex)
//                     .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                     .alignItems(NewArch::AlignItems::Center)
//                 (
//                     text(U"842 reactions   54 comments").fontSize(S::pt(12)).color(simd_float4{0.47,0.52,0.57,1.0}),
//                     div()
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .flexGap(S::px(8))
//                     (
//                         div(S::px(104), S::px(36), simd_float4{0.1,0.45,0.77,1.0})
//                             .cornerRadius(S::px(18))
//                             .display(NewArch::Display::Flex)
//                             .alignItems(NewArch::AlignItems::Center)
//                             .justifyContent(NewArch::JustifyContent::Center)
//                             .addEventListener(EventType::MouseDown, onClick)
//                         (
//                             text(U"Follow").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div(S::px(104), S::px(36), simd_float4{0.95,0.96,0.98,1.0})
//                             .cornerRadius(S::px(18))
//                             .borderColor(simd_float4{0.84,0.87,0.9,1.0})
//                             .borderWidth(S::px(1))
//                             .display(NewArch::Display::Flex)
//                             .alignItems(NewArch::AlignItems::Center)
//                             .justifyContent(NewArch::JustifyContent::Center)
//                         (
//                             text(U"Comment").fontSize(S::pt(13)).color(simd_float4{0.27,0.33,0.38,1.0})
//                         )
//                     )
//                 )
//             )
//         ),

//         div().gridColumn(3, 4).gridRow(2, 3)
//             .display(NewArch::Display::Flex)
//             .flexDirection(NewArch::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(NewArch::Display::Flex)
//                 .flexDirection(NewArch::FlexDirection::Col)
//                 .flexGap(S::px(12))
//             (
//                 text(U"LinkedIn News").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div().display(NewArch::Display::Flex).flexDirection(NewArch::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text(U"Renderer benchmarks are up").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text(U"Top story   1,204 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(NewArch::Display::Flex).flexDirection(NewArch::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text(U"More teams are testing native grids").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text(U"Trending   884 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(NewArch::Display::Flex).flexDirection(NewArch::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text(U"UI infra hiring cools, specialists still win").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text(U"3h ago   642 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 )
//             ),
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(NewArch::Display::Flex)
//                 .flexDirection(NewArch::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 text(U"Add to your feed").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div()
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                     .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .flexGap(S::px(10))
//                     (
//                         div(S::px(42), S::px(42), simd_float4{0.82,0.51,0.19,1.0}).cornerRadius(S::px(21)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center).justifyContent(NewArch::JustifyContent::Center)
//                         (
//                             text(U"AK").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(NewArch::Display::Flex).flexDirection(NewArch::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text(U"Ada Kim").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text(U"Graphics engineer").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center).justifyContent(NewArch::JustifyContent::Center)
//                     (
//                         text(U"+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 ),
//                 div()
//                     .display(NewArch::Display::Flex)
//                     .alignItems(NewArch::AlignItems::Center)
//                     .justifyContent(NewArch::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(NewArch::Display::Flex)
//                         .alignItems(NewArch::AlignItems::Center)
//                         .flexGap(S::px(10))
//                     (
//                         div(S::px(42), S::px(42), simd_float4{0.26,0.56,0.39,1.0}).cornerRadius(S::px(21)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center).justifyContent(NewArch::JustifyContent::Center)
//                         (
//                             text(U"LM").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(NewArch::Display::Flex).flexDirection(NewArch::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text(U"Layout Monthly").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text(U"Newsletter").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(NewArch::Display::Flex).alignItems(NewArch::AlignItems::Center).justifyContent(NewArch::JustifyContent::Center)
//                     (
//                         text(U"+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 )
//             )
//         )
//     );

}
