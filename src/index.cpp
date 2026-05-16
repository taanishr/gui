#include "index.hpp"
#include "events.hpp"
#include "new_arch.hpp"
#include "sizing.hpp"
#include <print>
#include <simd/vector_types.h>

const std::string Arial = "/System/Library/Fonts/Supplemental/Arial.ttf";
const std::string ArialBold = "/System/Library/Fonts/Supplemental/Arial Bold.ttf";

static int count = 0;


auto index() -> void {
    using gui::div;
    using gui::image;
    using gui::svg;
    using gui::text;
    using runtime::Event;

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
    // using S = gui::Size;

    // // Grid demo: holy grail layout
    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
    //     .display(gui::Display::Grid)
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
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Center)
    //         .justifyContent(gui::JustifyContent::Center)
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
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Center)
    //         .justifyContent(gui::JustifyContent::Center)
    //     (
    //         text(U"Footer").fontSize(S::pt(14)).color(simd_float4{1,1,1,1})
    //     )
    // );

   



    
//     div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
//     .display(gui::Display::Flex)
//     .paddingTop(gui::Size::px(32.0))
//     .flexDirection(gui::FlexDirection::Col)
// (
//     // Header
//     div(gui::Size::percent(1.0), gui::Size::px(56), simd_float4{1.0,1.0,1.0,1.0})
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .paddingLeft(gui::Size::px(16))
//         .paddingRight(gui::Size::px(16))
//         .borderColor(simd_float4{0.88,0.88,0.88,1.0})
//         .borderWidth(gui::Size::px(1.0))
//         .flexShrink(gui::Size::px(0.0))
//     (
//         // Left
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             div(gui::Size::px(40), gui::Size::px(40), simd_float4{0.96,0.96,0.96,1.0})
//                 .cornerRadius(gui::Size::px(20))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::Center)
//             (
//                 text(U"=").fontSize(gui::Size::pt(18)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .height(gui::Size::px(32))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"You: San Francisco").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
//             )
//         ),
//         // Right
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .height(gui::Size::px(36))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"Import").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//                 .height(gui::Size::px(36))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"Add trip").fontSize(gui::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     ),
//     // Body
//     div()
//         .color(simd_float4{0.0,0.0,0.0,0.0})
//         .display(gui::Display::Flex)
//         .flexGrow(gui::Size::px(1))
//     (
//         // Sidebar
//         div()
//             // .width(gui::Size::px(160.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .borderColor(simd_float4{0.88,0.88,0.88,1.0})
//             .borderWidth(gui::Size::px(1.0))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(gui::Size::px(4))
//             .padding(gui::Size::px(12))
//             .flexShrink(gui::Size::px(0.0))
//         (
//             text(U"TRIPS").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .paddingTop(gui::Size::px(8))
//                 .paddingBottom(gui::Size::px(8))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(2))
//             (
//                 text(U"Tokyo & Kyoto").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text(U"Mar 10 - Mar 24").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//             ),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .paddingTop(gui::Size::px(8))
//                 .paddingBottom(gui::Size::px(8))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(2))
//             (
//                 text(U"NYC Weekend").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text(U"Apr 4 - Apr 7").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//             ),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .paddingTop(gui::Size::px(8))
//                 .paddingBottom(gui::Size::px(8))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(2))
//             (
//                 text(U"London + Paris").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text(U"Jun 1 - Jun 12").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//             )
//         ),
//         // Main content
//         div()
//             .color(simd_float4{0.97,0.97,0.97,1.0})
//             .flexGrow(gui::Size::px(1))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .padding(gui::Size::px(24))
//             .flexGap(gui::Size::px(12))
//         (
//             // Title row
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(2))
//                 (
//                     text(U"Tokyo & Kyoto").fontSize(gui::Size::pt(20)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Mar 10 - Mar 24 · 14 days").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.96,0.96,0.96,1.0})
//                     .paddingLeft(gui::Size::px(16))
//                     .paddingRight(gui::Size::px(16))
//                     .height(gui::Size::px(32))
//                     .cornerRadius(gui::Size::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Edit").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//             ),
//             // Flight
//             div(gui::Size::percent(1.0), gui::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(12))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(4))
//                 (
//                     div()
//                         .color(simd_float4{0.0,0.0,0.0,0.0})
//                         .display(gui::Display::Flex)
//                         .flexGap(gui::Size::px(4))
//                         .alignItems(gui::AlignItems::Center)
//                     (
//                         svg("/Users/treja/projects/gui/assets/plane.svg")
//                             .width(gui::Size::px(22))
//                             .height(gui::Size::px(22)),
//                         text(U"SFO -> NRT").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     )
//                     ,text(U"Mar 10 · United 837 · 11h 30m").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.93,0.88,1.0,1.0})
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .height(gui::Size::px(24))
//                     .cornerRadius(gui::Size::px(12))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"[Flight]").fontSize(gui::Size::pt(11)).color(simd_float4{0.4,0.3,1.0,1.0})
//                 )
//             ),
//             // Hotel
//             div(gui::Size::percent(1.0), gui::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(12))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(4))
//                 (
//                     text(U"Park Hyatt Tokyo").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Mar 11 - Mar 17 · 6 nights").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.85,0.93,1.0,1.0})
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .height(gui::Size::px(24))
//                     .cornerRadius(gui::Size::px(12))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"[Hotel]").fontSize(gui::Size::pt(11)).color(simd_float4{0.2,0.6,0.9,1.0})
//                 )
//             ),
//             // Train
//             div(gui::Size::percent(1.0), gui::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(12))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(4))
//                 (
//                     text(U"Tokyo -> Kyoto").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Mar 17 · Shinkansen · 2h 15m").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.85,1.0,0.9,1.0})
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .height(gui::Size::px(24))
//                     .cornerRadius(gui::Size::px(12))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"[Train]").fontSize(gui::Size::pt(11)).color(simd_float4{0.1,0.7,0.4,1.0})
//                 )
//             )
//         )
//         // Detail panel
//         ,div()
//             // .width(gui::Size::px(200.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .borderColor(simd_float4{0.88,0.88,0.88,1.0})
//             .borderWidth(gui::Size::px(1.0))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(gui::Size::px(16))
//             .padding(gui::Size::px(16))
//             .flexShrink(gui::Size::px(0.0))
//         (
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 text(U"Flight Details").fontSize(gui::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text(U"x").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(12))
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text(U"Flight").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text(U"UA 837").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text(U"Departs").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text(U"10:45 AM").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text(U"Arrives").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text(U"3:15 PM +1").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text(U"Duration").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text(U"11h 30m").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text(U"Seat").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text(U"42A").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text(U"Class").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text(U"Economy").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 )
//             ),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{0.97,0.97,0.97,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .padding(gui::Size::px(12))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(4))
//             (
//                 text(U"Local time at destination").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                 text(U"3:15 PM JST (UTC+9)").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//             )
//         )
//     )
// );

//    div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::Center)
//     (
//         div()
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(16))
//             .padding(gui::Size::px(12))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .alignItems(gui::AlignItems::Center) // expands maxWidth for... some reason?
//             .flexGap(gui::Size::px(12))
//         (
//             image("/Users/treja/Downloads/sf90.jpg", gui::Size::px(80), gui::Size::px(80))
//                 .cornerRadius(gui::Size::percent(0.5))
//             ,text(U"Sarah Johnson")
//                 .fontSize(gui::Size::pt(18))
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//             ,text(U"Product Designer @ Figma")
//                 .fontSize(gui::Size::pt(13))
//                 .color(simd_float4{0.5,0.5,0.5,1.0})
//             ,//
//             div()
//                 .width(gui::Size::percent(1.0)) // this is broken now... percent sizing leads to this being full width of grandparent when html doesnt do that
//                 .height(gui::Size::px(50))
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .justifyContent(gui::JustifyContent::SpaceAround)
//             (
//                 div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"284").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     ,text(U"Posts").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//                 ,div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"12.4k").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Followers").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"891").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text(U"Following").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//             )
//             ,
//             div(gui::Size::px(120), gui::Size::px(40), simd_float4{0.4,0.3,1.0,1.0})
//                 .cornerRadius(gui::Size::px(20))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::Center)
//                 .addEventListener(EventType::MouseDown, onClick)
//             (
//                 text(U"Follow").fontSize(gui::Size::pt(14)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     );



// // Dashboard layout: analytics overview
// div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{0.96,0.96,0.97,1.0})
//     .display(gui::Display::Grid)
//     .gridTemplateColumns({gui::Size::px(220), gui::Size::fr(1), gui::Size::fr(1), gui::Size::fr(1)})
//     .gridTemplateRows({gui::Size::px(56), gui::Size::px(120), gui::Size::fr(1), gui::Size::px(44)})
//     .gridColumnGap(gui::Size::px(10))
//     .gridRowGap(gui::Size::px(10))
//     .padding(gui::Size::px(10))
//     .paddingTop(gui::Size::px(32))
// (
//     // ── Topbar: columns 1-5, row 1 ──
//     div().gridColumn(1, 5).gridRow(1, 2)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .paddingLeft(gui::Size::px(20))
//         .paddingRight(gui::Size::px(20))
//     (
//         text(U"Analytics").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .height(gui::Size::px(32))
//                 .paddingLeft(gui::Size::px(14))
//                 .paddingRight(gui::Size::px(14))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"Last 30 days").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//                 .height(gui::Size::px(32))
//                 .paddingLeft(gui::Size::px(14))
//                 .paddingRight(gui::Size::px(14))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"Export").fontSize(gui::Size::pt(12)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     ),

//     // ── Stat card: Revenue ──
//     div().gridColumn(2, 3).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//     (
//         text(U"Revenue").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"$48,230").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.85,1.0,0.9,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"+12.4%").fontSize(gui::Size::pt(10)).color(simd_float4{0.1,0.7,0.4,1.0})
//             )
//         ),
//         text(U"vs $42,900 last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Stat card: Users ──
//     div().gridColumn(3, 4).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//     (
//         text(U"Active Users").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"8,412").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.93,0.88,1.0,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"+3.1%").fontSize(gui::Size::pt(10)).color(simd_float4{0.4,0.3,1.0,1.0})
//             )
//         ),
//         text(U"vs 8,160 last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Stat card: Churn ──
//     div().gridColumn(4, 5).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//     (
//         text(U"Churn Rate").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"2.3%").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{1.0,0.9,0.88,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text(U"-0.4%").fontSize(gui::Size::pt(10)).color(simd_float4{0.9,0.3,0.2,1.0})
//             )
//         ),
//         text(U"vs 2.7% last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Nav sidebar: column 1, rows 2-4 ──
//     div().gridColumn(1, 2).gridRow(2, 4)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(12))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(gui::Size::px(4))
//     (
//         text(U"NAVIGATION").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{0.1,0.1,0.1,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"▪").fontSize(gui::Size::pt(10)).color(simd_float4{1.0,1.0,1.0,1.0}),
//             text(U"Overview").fontSize(gui::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Revenue").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Users").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Reports").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text(U"▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Settings").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         )
//     ),

//     // ── Chart area: columns 2-4, row 3 ──
//     div().gridColumn(2, 5).gridRow(3, 4)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(gui::Size::px(12))
//     (
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .justifyContent(gui::JustifyContent::SpaceBetween)
//         (
//             text(U"Revenue over time").fontSize(gui::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .flexGap(gui::Size::px(6))
//         (
//                 div()
//                     .color(simd_float4{0.96,0.96,0.96,1.0})
//                     .height(gui::Size::px(26))
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .cornerRadius(gui::Size::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Monthly").fontSize(gui::Size::pt(11)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.1,0.1,0.1,1.0})
//                     .height(gui::Size::px(26))
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .cornerRadius(gui::Size::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Weekly").fontSize(gui::Size::pt(11)).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .flexGrow(gui::Size::px(1))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::FlexEnd)
//             .flexGap(gui::Size::px(6))
//         (
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.61)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.78)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.50)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.89)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.72)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(1.00)).color(simd_float4{0.4,0.3,1.0,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.83)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))()
//         )
//     ),

//     // ── Footer: all columns, row 4 ──
//     div().gridColumn(1, 5).gridRow(4, 5)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .paddingLeft(gui::Size::px(20))
//         .paddingRight(gui::Size::px(20))
//     (
//         text(U"Last synced: Apr 17, 2026 at 9:41 AM").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(16))
//         (
//             text(U"Privacy").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Terms").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text(U"Help").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//         )
//     )
// );

// using S = gui::Size;
// div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.96,0.98,1.0})
//         .display(gui::Display::Grid)
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
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .justifyContent(gui::JustifyContent::SpaceBetween)
//             .paddingLeft(S::px(18))
//             .paddingRight(S::px(18))
//         (
//             div()
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .flexGap(S::px(12))
//             (
//                 div(S::px(34), S::px(34), simd_float4{0.04,0.4,0.67,1.0})
//                     .cornerRadius(S::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::Center)
//                 (
//                     text(U"in").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.94,0.97,1.0,1.0})
//                     .height(S::px(38))
//                     .paddingLeft(S::px(14))
//                     .paddingRight(S::px(64))
//                     .cornerRadius(S::px(19))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Search").fontSize(S::pt(13)).color(simd_float4{0.41,0.49,0.56,1.0})
//                 )
//             ),
//             div()
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .flexGap(S::px(10))
//             (
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Home").fontSize(S::pt(13)).color(simd_float4{0.12,0.16,0.2,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"My Network").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Jobs").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Messages").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(14)).paddingRight(S::px(14)).height(S::px(36)).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(18)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"Try Premium").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),

//         div().gridColumn(1, 2).gridRow(2, 3)
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//             (
//                 div()
//                     .height(S::px(74))
//                     .color(simd_float4{0.72,0.84,0.95,1.0})
//                     .cornerRadius(S::px(10))(),
//                 div()
//                     .paddingLeft(S::px(18))
//                     .paddingRight(S::px(18))
//                     .paddingBottom(S::px(18))
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                     .flexGap(S::px(8))
//                 (
//                     div(S::px(84), S::px(84), simd_float4{0.1,0.45,0.77,1.0})
//                         .cornerRadius(S::px(42))
//                         .marginTop(S::px(-42))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .justifyContent(gui::JustifyContent::Center)
//                     (
//                         text(U"TR").fontSize(S::pt(24)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     text(U"Taanish Reja").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{0.09,0.12,0.16,1.0}),
//                     text(U"Building a GPU-native UI kit").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                     div().height(S::px(1)).color(simd_float4{0.91,0.93,0.95,1.0})(),
//                     div()
//                         .display(gui::Display::Flex)
//                         .justifyContent(gui::JustifyContent::SpaceBetween)
//                     (
//                         text(U"Profile viewers").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text(U"128").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div()
//                         .display(gui::Display::Flex)
//                         .justifyContent(gui::JustifyContent::SpaceBetween)
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
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
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
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .flexGap(S::px(12))
//                 (
//                     div(S::px(52), S::px(52), simd_float4{0.13,0.52,0.81,1.0})
//                         .cornerRadius(S::px(26))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .justifyContent(gui::JustifyContent::Center)
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
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGrow(S::px(1))
//                     (
//                         text(U"Start a post about the grid stress test").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text(U"Photo").fontSize(S::pt(12)).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text(U"Video").fontSize(S::pt(12)).color(simd_float4{0.09,0.58,0.37,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
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
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(12))
//                     (
//                         div(S::px(52), S::px(52), simd_float4{0.18,0.18,0.2,1.0})
//                             .cornerRadius(S::px(26))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                         (
//                             text(U"MS").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div()
//                             .display(gui::Display::Flex)
//                             .flexDirection(gui::FlexDirection::Col)
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
//                     .display(gui::Display::Grid)
//                     .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1), S::fr(1)})
//                     .gridTemplateRows({S::px(64), S::fr(1), S::px(46)})
//                     .gridColumnGap(S::px(10))
//                     .gridRowGap(S::px(10))
//                 (
//                     div().gridColumn(1, 5).gridRow(1, 2).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(10))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text(U"Feed card inside a feed card").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     div().gridColumn(1, 2).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(2, 4).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(4, 5).gridRow(2, 3).color(simd_float4{0.84,0.91,0.98,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(1, 5).gridRow(3, 4).color(simd_float4{0.16,0.2,0.24,1.0}).cornerRadius(S::px(10))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text(U"Nested grid preview").fontSize(S::pt(12)).color(simd_float4{0.92,0.94,0.97,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text(U"842 reactions   54 comments").fontSize(S::pt(12)).color(simd_float4{0.47,0.52,0.57,1.0}),
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(8))
//                     (
//                         div(S::px(104), S::px(36), simd_float4{0.1,0.45,0.77,1.0})
//                             .cornerRadius(S::px(18))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                             .addEventListener(EventType::MouseDown, onClick)
//                         (
//                             text(U"Follow").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div(S::px(104), S::px(36), simd_float4{0.95,0.96,0.98,1.0})
//                             .cornerRadius(S::px(18))
//                             .borderColor(simd_float4{0.84,0.87,0.9,1.0})
//                             .borderWidth(S::px(1))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                         (
//                             text(U"Comment").fontSize(S::pt(13)).color(simd_float4{0.27,0.33,0.38,1.0})
//                         )
//                     )
//                 )
//             )
//         ),

//         div().gridColumn(3, 4).gridRow(2, 3)
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(12))
//             (
//                 text(U"LinkedIn News").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text(U"Renderer benchmarks are up").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text(U"Top story   1,204 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text(U"More teams are testing native grids").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text(U"Trending   884 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
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
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 text(U"Add to your feed").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(10))
//                     (
//                         div(S::px(42), S::px(42), simd_float4{0.82,0.51,0.19,1.0}).cornerRadius(S::px(21)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                         (
//                             text(U"AK").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text(U"Ada Kim").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text(U"Graphics engineer").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                     (
//                         text(U"+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(10))
//                     (
//                         div(S::px(42), S::px(42), simd_float4{0.26,0.56,0.39,1.0}).cornerRadius(S::px(21)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                         (
//                             text(U"LM").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text(U"Layout Monthly").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text(U"Newsletter").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                     (
//                         text(U"+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 )
//             )
//         )
//     );

    using S = gui::Size;

    // Dark music player — scrollable playlist (left) + nested scrollable lyrics (right)
    div(S::percent(1.0), S::percent(1.0), simd_float4{0.09,0.09,0.11,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .paddingTop(S::px(28))
    (
        // ── Now Playing bar ──
        div(S::percent(1.0), S::px(64), simd_float4{0.13,0.13,0.16,1.0})
            .borderColor(simd_float4{0.22,0.22,0.26,1.0})
            .borderWidth(S::px(1))
            .flexShrink(S::px(0.0))
            .display(gui::Display::Flex)
            .alignItems(gui::AlignItems::Center)
            .justifyContent(gui::JustifyContent::SpaceBetween)
            .paddingLeft(S::px(24))
            .paddingRight(S::px(24))
        (
            div()
                .display(gui::Display::Flex)
                .alignItems(gui::AlignItems::Center)
                .flexGap(S::px(12))
            (
                div(S::px(40), S::px(40), simd_float4{0.18,0.72,0.56,1.0})
                    .cornerRadius(S::px(8))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::Center)
                (
                    text(U"♪").fontSize(S::pt(20)).color(simd_float4{1.0,1.0,1.0,1.0})
                ),
                div()
                    .display(gui::Display::Flex)
                    .flexDirection(gui::FlexDirection::Col)
                    .flexGap(S::px(3))
                (
                    text(U"Endless Reverie").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.92,0.92,0.94,1.0}),
                    text(U"Glass Prism  ·  Mirrors").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
                )
            ),
            div()
                .display(gui::Display::Flex)
                .alignItems(gui::AlignItems::Center)
                .flexGap(S::px(16))
            (
                div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                    .cornerRadius(S::px(16))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::Center)
                (
                    text(U"|<").fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})
                ),
                div(S::px(44), S::px(44), simd_float4{0.18,0.72,0.56,1.0})
                    .cornerRadius(S::px(22))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::Center)
                (
                    text(U"||").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
                ),
                div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                    .cornerRadius(S::px(16))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::Center)
                (
                    text(U">|").fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})
                )
            ),
            div()
                .display(gui::Display::Flex)
                .alignItems(gui::AlignItems::Center)
                .flexGap(S::px(10))
            (
                text(U"2:14").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0}),
                div(S::px(100), S::px(4), simd_float4{0.24,0.24,0.28,1.0})
                    .cornerRadius(S::px(2))
                (
                    div(S::px(48), S::px(4), simd_float4{0.18,0.72,0.56,1.0})
                        .cornerRadius(S::px(2))()
                ),
                text(U"4:38").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
            )
        ),
        // ── Body ──
        div(S::percent(1.0), S::percent(1.0), simd_float4{0.0,0.0,0.0,0.0})
            .display(gui::Display::Flex)
            .flexGrow(S::px(1))
        (
            // Left: scrollable playlist
            div(S::px(260), S::percent(1.0), simd_float4{0.11,0.11,0.14,1.0})
                .borderColor(simd_float4{0.20,0.20,0.24,1.0})
                .borderWidth(S::px(1))
                .flexShrink(S::px(0.0))
                .paddingTop(S::px(16))
                .paddingBottom(S::px(16))
                .overflow(gui::Overflow::Scroll)
                .display(gui::Display::Flex)
                .flexDirection(gui::FlexDirection::Col)
                .flexGap(S::px(1))
            (
                div()
                    .paddingLeft(S::px(16))
                    .paddingRight(S::px(16))
                    .paddingBottom(S::px(10))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    text(U"PLAYLIST").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
                    text(U"12 tracks").fontSize(S::pt(10)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 01 – active
                div(S::percent(1.0), S::px(52), simd_float4{0.14,0.22,0.20,1.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"01").fontSize(S::pt(11)).color(simd_float4{0.18,0.72,0.56,1.0}),
                        text(U"Endless Reverie").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.18,0.72,0.56,1.0})
                    ),
                    text(U"4:38").fontSize(S::pt(12)).color(simd_float4{0.18,0.72,0.56,1.0})
                ),
                // 02
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"02").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Crystalline").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"3:52").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 03
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"03").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Pale Shore").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"5:14").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 04
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"04").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Inversion").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"4:07").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 05
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"05").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Soft Architecture").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"6:21").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 06
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"06").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Between Frames").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"3:44").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 07
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"07").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Nocturne Loop").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"4:58").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 08
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"08").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Refract").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"3:30").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 09
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"09").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Diffusion").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"5:02").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 10
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"10").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Afterimage").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"4:15").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 11
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"11").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Threshold").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"7:03").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 12
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text(U"12").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text(U"Dissolve").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text(U"4:49").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                )
            ),
            // Right: album info + scrollable lyrics
            div(S::percent(1.0), S::percent(1.0), simd_float4{0.10,0.10,0.12,1.0})
                .flexGrow(S::px(1))
                .display(gui::Display::Flex)
                .flexDirection(gui::FlexDirection::Col)
                .padding(S::px(32))
                .flexGap(S::px(20))
            (
                // Album banner
                div(S::percent(1.0), S::px(160), simd_float4{0.0,0.0,0.0,0.0})
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .flexGap(S::px(24))
                    .flexShrink(S::px(0.0))
                (
                    div(S::px(152), S::px(152), simd_float4{0.18,0.72,0.56,1.0})
                        .cornerRadius(S::px(14))
                        .display(gui::Display::Flex)
                        .alignItems(gui::AlignItems::Center)
                        .justifyContent(gui::JustifyContent::Center)
                    (
                        text(U"♫").fontSize(S::pt(52)).color(simd_float4{1.0,1.0,1.0,1.0})
                    ),
                    div()
                        .display(gui::Display::Flex)
                        .flexDirection(gui::FlexDirection::Col)
                        .flexGap(S::px(6))
                    (
                        text(U"ALBUM").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
                        text(U"Mirrors").fontSize(S::pt(30)).font(ArialBold).color(simd_float4{0.92,0.92,0.94,1.0}),
                        text(U"Glass Prism").fontSize(S::pt(16)).color(simd_float4{0.18,0.72,0.56,1.0}),
                        text(U"2024  ·  Ambient  ·  12 tracks").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0}),
                        div()
                            .display(gui::Display::Flex)
                            .flexGap(S::px(10))
                            .paddingTop(S::px(8))
                        (
                            div(S::px(96), S::px(32), simd_float4{0.18,0.72,0.56,1.0})
                                .cornerRadius(S::px(16))
                                .display(gui::Display::Flex)
                                .alignItems(gui::AlignItems::Center)
                                .justifyContent(gui::JustifyContent::Center)
                            (
                                text(U"Play all").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
                            ),
                            div(S::px(96), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                                .cornerRadius(S::px(16))
                                .display(gui::Display::Flex)
                                .alignItems(gui::AlignItems::Center)
                                .justifyContent(gui::JustifyContent::Center)
                            (
                                text(U"Shuffle").fontSize(S::pt(13)).color(simd_float4{0.72,0.72,0.78,1.0})
                            )
                        )
                    )
                ),
                // Divider
                div(S::percent(1.0), S::px(1), simd_float4{0.20,0.20,0.24,1.0})
                    .flexShrink(S::px(0.0))(),
                // Lyrics label row
                div()
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::SpaceBetween)
                    .flexShrink(S::px(0.0))
                (
                    text(U"LYRICS").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
                    text(U"Endless Reverie").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
                ),
                // Scrollable lyrics
                div(S::percent(1.0), S::percent(1.0), simd_float4{0.13,0.13,0.16,1.0})
                    .cornerRadius(S::px(12))
                    .padding(S::px(22))
                    .overflow(gui::Overflow::Scroll)
                    .display(gui::Display::Flex)
                    .flexDirection(gui::FlexDirection::Col)
                    .flexGap(S::px(7))
                    .flexGrow(S::px(1))
                (
                    text(U"Through the glass, a world apart,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"endless echoes fill the dark.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"Fractures in the silver light —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"Hold a breath and feel the weight").fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0}),
                    text(U"of every word you couldn't say,").fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0}),
                    text(U"mirrored back in shades of grey.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"Soft light bends around your face,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"I chase the outline, lose the trace.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"The mirror holds what time erased —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"Still you linger in the seams,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"half-remembered, half in dreams.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"I reach — the surface bends and gleams.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"(Instrumental)").fontSize(S::pt(14)).color(simd_float4{0.35,0.35,0.42,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"The glass grows cold, the echo fades,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"and all that's left is what remains —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text(U"a shape of light, a broken name.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text(U"Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text(U"a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})
                )
            )
        )
    );

}
