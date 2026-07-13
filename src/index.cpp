#include "index.hpp"
#include "events.hpp"
#include "fonts.hpp"
#include "new_arch.hpp"
#include "sizing.hpp"
#include <print>
#include <simd/vector_types.h>

static int count = 0;


auto index() -> void {
    using gui::div;
    using gui::image;
    using gui::svg;
    using gui::text;
    using runtime::Event;

    auto onClick = [](auto& node, Event& event){
        count += 1;

        if (count % 2 == 0) {
            node.color(simd_float4{1.0,0.0,0.0,1.0});
        }else {
            node.color(simd_float4{0.0,0.0,1.0,1.0});
        }

        std::println("hello world {}", count);
    };

    // div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .borderColor(simd_float4{0.77,0.71,1.0,1.0})
    //     .borderWidth(gui::Size::px(1.0))
    // (
    //      div(gui::Size::percent(0.2), gui::Size::percent(1.0), simd_float4{1.0,0.5,1.0,0.8})(
    //         div(gui::Size::px(60), gui::Size::px(30), simd_float4{0.498,0.0,1.0,1.0})
    //             .marginTop(30)
    //             .marginLeft(gui::Size::autoSize())
    //             .marginRight(gui::Size::autoSize())
    //             .cornerRadius(gui::Size::px(7.5))
    //             .paddingLeft(gui::Size::px(9.0))
    //             .paddingTop(gui::Size::px(4.5))
    //             .borderColor(simd_float4{0.77,0.71,1.0,1.0})
    //             .borderWidth(gui::Size::px(1.0))
    //             .addEventListener(runtime::EventType::MouseDown, onClick)
    //         (
    //             text("Startfsd")
    //                 .fontSize(gui::Size::pt(48.0))
    //                 .font(Arial)
    //                 .marginLeft(10)
    //                 .marginRight(10)
    //                 .color(simd_float4{0,0,0,1})
    //                 ,
    //             text("fsdfsdfsda   dads sdsfsdsds")
    //                 .fontSize(gui::Size::pt(48.0))
    //                 .font(Arial)
    //                 .color(simd_float4{0,0,0,1})
    //         ),
    //         div(
    //             gui::Size::px(60), gui::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
    //         ).marginTop(10)
    //      )
    // );

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
    //         text("Header").fontSize(S::pt(20)).color(simd_float4{1,1,1,1})
    //     ),

    //     // Left sidebar
    //     div().gridColumn(1, 2).gridRow(2, 3)
    //         .color(simd_float4{0.9,0.9,0.95,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text("Sidebar").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
    //     ),

    //     // Main content
    //     div().gridColumn(2, 3).gridRow(2, 3)
    //         .color(simd_float4{1.0,1.0,1.0,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text("Main Content").fontSize(S::pt(14)).color(simd_float4{0.1,0.1,0.1,1})
    //     ),

    //     // Right sidebar
    //     div().gridColumn(3, 4).gridRow(2, 3)
    //         .color(simd_float4{0.9,0.9,0.95,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text("Panel").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
    //     ),

    //     // Footer — spans all 3 columns
    //     div().gridColumn(1, 4).gridRow(3, 4)
    //         .color(simd_float4{0.3,0.3,0.35,1.0})
    //         .cornerRadius(S::px(8))
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Center)
    //         .justifyContent(gui::JustifyContent::Center)
    //     (
    //         text("Footer").fontSize(S::pt(14)).color(simd_float4{1,1,1,1})
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
//                 text("=").fontSize(gui::Size::pt(18)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                 text("You: San Francisco").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                 text("Import").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                 text("Add trip").fontSize(gui::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     ),
//     // Body
//     div()
//         .color(simd_float4{0.0,0.0,0.0,0.0})
//         .display(gui::Display::Flex)
//         .flexGrow(gui::Size::px(1))
//         .overflow(gui::Overflow::Scroll)
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
//             text("TRIPS").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
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
//                 text("Tokyo & Kyoto").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("Mar 10 - Mar 24").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                 text("NYC Weekend").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("Apr 4 - Apr 7").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                 text("London + Paris").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("Jun 1 - Jun 12").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//             .overflow(gui::Overflow::Scroll)
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
//                     text("Tokyo & Kyoto").fontSize(gui::Size::pt(20)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Mar 10 - Mar 24 · 14 days").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                     text("Edit").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                         text("SFO -> NRT").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     )
//                     ,text("Mar 10 · United 837 · 11h 30m").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                     text("[Flight]").fontSize(gui::Size::pt(11)).color(simd_float4{0.4,0.3,1.0,1.0})
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
//                     text("Park Hyatt Tokyo").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Mar 11 - Mar 17 · 6 nights").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                     text("[Hotel]").fontSize(gui::Size::pt(11)).color(simd_float4{0.2,0.6,0.9,1.0})
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
//                     text("Tokyo -> Kyoto").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Mar 17 · Shinkansen · 2h 15m").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                     text("[Train]").fontSize(gui::Size::pt(11)).color(simd_float4{0.1,0.7,0.4,1.0})
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
//             .overflow(gui::Overflow::Scroll)
//         (
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 text("Flight Details").fontSize(gui::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("x").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                     text("Flight").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("UA 837").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Departs").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("10:45 AM").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Arrives").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("3:15 PM +1").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Duration").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("11h 30m").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Seat").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("42A").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Class").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("Economy").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
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
//                 text("Local time at destination").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                 text("3:15 PM JST (UTC+9)").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
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
//             ,text("Sarah Johnson")
//                 .fontSize(gui::Size::pt(18))
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//             ,text("Product Designer @ Figma")
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
//                     text("284").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     ,text("Posts").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//                 ,div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("12.4k").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Followers").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("891").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Following").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//             )
//             ,
//             div(gui::Size::px(120), gui::Size::px(40), simd_float4{0.4,0.3,1.0,1.0})
//                 .cornerRadius(gui::Size::px(20))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::Center)
//                 .addEventListener(runtime::EventType::MouseDown, onClick)
//             (
//                 text("Follow").fontSize(gui::Size::pt(14)).color(simd_float4{1.0,1.0,1.0,1.0})
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
//         text("Analytics").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
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
//                 text("Last 30 days").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                 text("Export").fontSize(gui::Size::pt(12)).color(simd_float4{1.0,1.0,1.0,1.0})
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
//         text("Revenue").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("$48,230").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.85,1.0,0.9,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("+12.4%").fontSize(gui::Size::pt(10)).color(simd_float4{0.1,0.7,0.4,1.0})
//             )
//         ),
//         text("vs $42,900 last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
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
//         text("Active Users").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("8,412").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.93,0.88,1.0,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("+3.1%").fontSize(gui::Size::pt(10)).color(simd_float4{0.4,0.3,1.0,1.0})
//             )
//         ),
//         text("vs 8,160 last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
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
//         text("Churn Rate").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("2.3%").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{1.0,0.9,0.88,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("-0.4%").fontSize(gui::Size::pt(10)).color(simd_float4{0.9,0.3,0.2,1.0})
//             )
//         ),
//         text("vs 2.7% last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
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
//         text("NAVIGATION").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
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
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{1.0,1.0,1.0,1.0}),
//             text("Overview").fontSize(gui::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
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
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Revenue").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
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
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Users").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
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
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Reports").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
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
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Settings").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
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
//             text("Revenue over time").fontSize(gui::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
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
//                     text("Monthly").fontSize(gui::Size::pt(11)).color(simd_float4{0.5,0.5,0.5,1.0})
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
//                     text("Weekly").fontSize(gui::Size::pt(11)).color(simd_float4{1.0,1.0,1.0,1.0})
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
//         text("Last synced: Apr 17, 2026 at 9:41 AM").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(16))
//         (
//             text("Privacy").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Terms").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Help").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
//                     text("in").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
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
//                     text("Search").fontSize(S::pt(13)).color(simd_float4{0.41,0.49,0.56,1.0})
//                 )
//             ),
//             div()
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .flexGap(S::px(10))
//             (
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Home").fontSize(S::pt(13)).color(simd_float4{0.12,0.16,0.2,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("My Network").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Jobs").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Messages").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(14)).paddingRight(S::px(14)).height(S::px(36)).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(18)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Try Premium").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
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
//                         text("TR").fontSize(S::pt(24)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     text("Taanish Reja").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{0.09,0.12,0.16,1.0}),
//                     text("Building a GPU-native UI kit").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                     div().height(S::px(1)).color(simd_float4{0.91,0.93,0.95,1.0})(),
//                     div()
//                         .display(gui::Display::Flex)
//                         .justifyContent(gui::JustifyContent::SpaceBetween)
//                     (
//                         text("Profile viewers").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text("128").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div()
//                         .display(gui::Display::Flex)
//                         .justifyContent(gui::JustifyContent::SpaceBetween)
//                     (
//                         text("Post impressions").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text("2,481").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
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
//                 text("Recent").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 text("#metal").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text("#layoutengines").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text("#cpp").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text("#rendering").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0})
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
//                         text("TR").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
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
//                         text("Start a post about the grid stress test").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text("Photo").fontSize(S::pt(12)).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text("Video").fontSize(S::pt(12)).color(simd_float4{0.09,0.58,0.37,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text("Write article").fontSize(S::pt(12)).color(simd_float4{0.79,0.47,0.08,1.0})
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
//                             text("MS").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div()
//                             .display(gui::Display::Flex)
//                             .flexDirection(gui::FlexDirection::Col)
//                             .flexGap(S::px(3))
//                         (
//                             text("Maya Stone").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                             text("Design systems at Northstar").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                             text("2h").fontSize(S::pt(11)).color(simd_float4{0.58,0.62,0.67,1.0})
//                         )
//                     ),
//                     text("...").fontSize(S::pt(16)).color(simd_float4{0.53,0.57,0.62,1.0})
//                 ),
//                 text("Spent the morning rebuilding our composer with a new grid shell and nested flex rows. The ergonomics are getting close.")
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
//                         text("Feed card inside a feed card").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     div().gridColumn(1, 2).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(2, 4).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(4, 5).gridRow(2, 3).color(simd_float4{0.84,0.91,0.98,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(1, 5).gridRow(3, 4).color(simd_float4{0.16,0.2,0.24,1.0}).cornerRadius(S::px(10))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text("Nested grid preview").fontSize(S::pt(12)).color(simd_float4{0.92,0.94,0.97,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("842 reactions   54 comments").fontSize(S::pt(12)).color(simd_float4{0.47,0.52,0.57,1.0}),
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
//                             text("Follow").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div(S::px(104), S::px(36), simd_float4{0.95,0.96,0.98,1.0})
//                             .cornerRadius(S::px(18))
//                             .borderColor(simd_float4{0.84,0.87,0.9,1.0})
//                             .borderWidth(S::px(1))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                         (
//                             text("Comment").fontSize(S::pt(13)).color(simd_float4{0.27,0.33,0.38,1.0})
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
//                 text("LinkedIn News").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text("Renderer benchmarks are up").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text("Top story   1,204 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text("More teams are testing native grids").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text("Trending   884 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text("UI infra hiring cools, specialists still win").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text("3h ago   642 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
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
//                 text("Add to your feed").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
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
//                             text("AK").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text("Ada Kim").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text("Graphics engineer").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                     (
//                         text("+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
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
//                             text("LM").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text("Layout Monthly").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text("Newsletter").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                     (
//                         text("+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 )
//             )
//         )
//     );






    using S = gui::Size;

    // Existing dark music player test.
    // Dark music player — scrollable playlist (left) + nested scrollable lyrics (right)
    /*
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
                    text("♪").fontSize(S::pt(20)).color(simd_float4{1.0,1.0,1.0,1.0})
                ),
                div()
                    .display(gui::Display::Flex)
                    .flexDirection(gui::FlexDirection::Col)
                    .flexGap(S::px(3))
                (
                    text("Endless Reverie").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.92,0.92,0.94,1.0}),
                    text("Glass Prism  ·  Mirrors").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
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
                    text("|<").fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})
                ),
                div(S::px(44), S::px(44), simd_float4{0.18,0.72,0.56,1.0})
                    .cornerRadius(S::px(22))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::Center)
                (
                    text("||").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
                ),
                div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                    .cornerRadius(S::px(16))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::Center)
                (
                    text(">|").fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})
                )
            ),
            div()
                .display(gui::Display::Flex)
                .alignItems(gui::AlignItems::Center)
                .flexGap(S::px(10))
            (
                text("2:14").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0}),
                div(S::px(100), S::px(4), simd_float4{0.24,0.24,0.28,1.0})
                    .cornerRadius(S::px(2))
                (
                    div(S::px(48), S::px(4), simd_float4{0.18,0.72,0.56,1.0})
                        .cornerRadius(S::px(2))()
                ),
                text("4:38").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
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
                    text("PLAYLIST").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
                    text("12 tracks").fontSize(S::pt(10)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 01 – active
                div(S::percent(1.0), S::px(52), simd_float4{0.14,0.22,0.20,1.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("01").fontSize(S::pt(11)).color(simd_float4{0.18,0.72,0.56,1.0}),
                        text("Endless Reverie").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.18,0.72,0.56,1.0})
                    ),
                    text("4:38").fontSize(S::pt(12)).color(simd_float4{0.18,0.72,0.56,1.0})
                ),
                // 02
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("02").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Crystalline").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("3:52").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 03
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("03").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Pale Shore").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("5:14").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 04
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("04").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Inversion").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("4:07").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 05
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("05").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Soft Architecture").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("6:21").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 06
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("06").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Between Frames").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("3:44").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 07
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("07").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Nocturne Loop").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("4:58").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 08
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("08").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Refract").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("3:30").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 09
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("09").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Diffusion").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("5:02").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 10
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("10").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Afterimage").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("4:15").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 11
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("11").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Threshold").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("7:03").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
                ),
                // 12
                div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                    .paddingLeft(S::px(16)).paddingRight(S::px(16))
                    .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                (
                    div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                    (
                        text("12").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
                        text("Dissolve").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
                    ),
                    text("4:49").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
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
                        text("♫").fontSize(S::pt(52)).color(simd_float4{1.0,1.0,1.0,1.0})
                    ),
                    div()
                        .display(gui::Display::Flex)
                        .flexDirection(gui::FlexDirection::Col)
                        .flexGap(S::px(6))
                    (
                        text("ALBUM").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
                        text("Mirrors").fontSize(S::pt(30)).font(ArialBold).color(simd_float4{0.92,0.92,0.94,1.0}),
                        text("Glass Prism").fontSize(S::pt(16)).color(simd_float4{0.18,0.72,0.56,1.0}),
                        text("2024  ·  Ambient  ·  12 tracks").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0}),
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
                                text("Play all").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
                            ),
                            div(S::px(96), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                                .cornerRadius(S::px(16))
                                .display(gui::Display::Flex)
                                .alignItems(gui::AlignItems::Center)
                                .justifyContent(gui::JustifyContent::Center)
                            (
                                text("Shuffle").fontSize(S::pt(13)).color(simd_float4{0.72,0.72,0.78,1.0})
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
                    text("LYRICS").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
                    text("Endless Reverie").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
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
                    text("Through the glass, a world apart,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("endless echoes fill the dark.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("Fractures in the silver light —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("Hold a breath and feel the weight").fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0}),
                    text("of every word you couldn't say,").fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0}),
                    text("mirrored back in shades of grey.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("Soft light bends around your face,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("I chase the outline, lose the trace.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("The mirror holds what time erased —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("Still you linger in the seams,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("half-remembered, half in dreams.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("I reach — the surface bends and gleams.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("(Instrumental)").fontSize(S::pt(14)).color(simd_float4{0.35,0.35,0.42,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("The glass grows cold, the echo fades,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("and all that's left is what remains —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    text("a shape of light, a broken name.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
                    div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                    text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
                    text("a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})
                )
            )
        )
    );
    */

    const std::string Devanagari = "/System/Library/Fonts/Supplemental/Devanagari Sangam MN.ttc";
    const std::string GeezaPro = "/System/Library/Fonts/GeezaPro.ttc";
    const auto background = simd_float4{0.07f, 0.08f, 0.10f, 1.0f};
    const auto panel = simd_float4{0.12f, 0.13f, 0.16f, 1.0f};
    const auto heading = simd_float4{0.45f, 0.80f, 1.0f, 1.0f};
    const auto body = simd_float4{0.92f, 0.93f, 0.96f, 1.0f};
    const auto note = simd_float4{0.58f, 0.62f, 0.70f, 1.0f};

    div(S::percent(1.0), S::percent(1.0), background)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(16))
        .padding(S::px(28))
        .overflow(gui::Overflow::Scroll)
    (
        text("HarfBuzz shaping coverage").font(ArialBold).fontSize(S::pt(24)).color(body),
        text("Kerning, ligatures, combining marks, and contextual script shaping")
            .fontSize(S::pt(13)).color(note),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("4. Kerning").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("AVATAR   To Wa Yo VA").font(Arial).fontSize(S::pt(34)).color(body),
            text("Look for tighter AV, To, Wa, and Yo pairs.").fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("5. Ligatures").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("office   affinity   ffi   fi   fl").font(Helvetica).fontSize(S::pt(34)).color(body),
            text("Helvetica should substitute fi/fl glyphs; ‘office’ should contain an fi ligature.")
                .fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("6a. Combining marks").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("decomposed:  cafe\xCC\x81   A\xCC\x8A   n\xCC\x83").font(Arial).fontSize(S::pt(34)).color(body),
            text("composed:    café   Å   ñ").font(Arial).fontSize(S::pt(34)).color(body),
            text("The two rows should have equivalent accent placement.").fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("6b. Arabic contextual shaping").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("السلام عليكم").font(GeezaPro).fontSize(S::pt(40)).color(body),
            text("Letters should join contextually; lam-alef should form a ligature.").fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("6c. Devanagari reordering and conjuncts").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("नमस्ते दुनिया").font(Devanagari).fontSize(S::pt(40)).color(body),
            text("The conjunct and pre-base vowel marks should be shaped and positioned.")
                .fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("7a. Mixed LTR and RTL runs").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("Hello السلام").font(Arial).fontSize(S::pt(30)).color(body),
            text("السلام Hello").font(Arial).fontSize(S::pt(30)).color(body),
            text("Each Arabic run should remain joined while the two scripts retain their reading order.")
                .fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("7b. Numbers and neutral punctuation").font(ArialBold).fontSize(S::pt(13)).color(heading),
            text("السلام 123 عليكم").font(Arial).fontSize(S::pt(30)).color(body),
            text("Hello (السلام) 123").font(Arial).fontSize(S::pt(30)).color(body),
            text("The digits should read 123 and the parentheses should enclose the Arabic word.")
                .fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("7c. Bidi resolution across text siblings").font(ArialBold).fontSize(S::pt(13)).color(heading),
            div()
            (
                text("Hello ").font(Arial).fontSize(S::pt(30)).color(body),
                text("السلام").font(GeezaPro).fontSize(S::pt(30)).color(body),
                text(" 123").font(Arial).fontSize(S::pt(30)).color(body)
            ),
            text("This is three sibling Text nodes and should match a single mixed-direction sentence.")
                .fontSize(S::pt(12)).color(note)
        ),

        div().width(S::percent(1.0)).color(panel).padding(S::px(18))
            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
        (
            text("7d. Mixed-direction wrapping").font(ArialBold).fontSize(S::pt(13)).color(heading),
            div().width(S::px(260))
            (
                text("Hello السلام 123 عليكم from a narrow mixed-direction line")
                    .font(Arial).fontSize(S::pt(24)).color(body)
            ),
            text("Every physical line should remain separate, joined, and non-overlapping.")
                .fontSize(S::pt(12)).color(note)
        )
    );

//     const auto alignmentSample = R"(Short line
// A considerably longer line that wraps inside the panel)";
//     const auto panelColor = simd_float4{0.94f, 0.95f, 0.97f, 1.0f};
//     const auto textColor = simd_float4{0.08f, 0.09f, 0.11f, 1.0f};
//     const auto labelColor = simd_float4{0.32f, 0.35f, 0.40f, 1.0f};

//     div(S::percent(1.0), S::percent(1.0), simd_float4{1.0f, 1.0f, 1.0f, 1.0f})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(S::px(12))
//         .padding(S::px(24))
//         .overflow(gui::Overflow::Scroll)
//     (
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Start").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Start)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Left").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Left)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Center").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Center)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Right").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Right)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         )
//     );



//     const auto sample = R"(Alpha   beta gamma delta epsilon zeta
// Second   line with Supercalifragilisticexpialidocious tail)";
//     const auto breakAllSample = "BreakAll: Supercalifragilisticexpialidocious0123456789";
//     const auto panelColor = simd_float4{0.94f, 0.95f, 0.97f, 1.0f};
//     const auto textColor = simd_float4{0.08f, 0.09f, 0.11f, 1.0f};
//     const auto labelColor = simd_float4{0.32f, 0.35f, 0.40f, 1.0f};

//     div(S::percent(1.0), S::percent(1.0), simd_float4{1.0f, 1.0f, 1.0f, 1.0f})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(S::px(12))
//         .padding(S::px(24))
//         .overflow(gui::Overflow::Scroll)
//     (
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Normal").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::Normal)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("NoWrap").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::NoWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Pre").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::Pre)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("PreWrap").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Normal + BreakAll").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(breakAllSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::Normal)
//                     .wordBreak(gui::WordBreak::BreakAll)
//             )
//         )
//     );


    // const auto overflowSample = "The quick brown fox jumps over the lazy dog";
    // const auto panelColor = simd_float4{0.94f, 0.95f, 0.97f, 1.0f};
    // const auto textColor = simd_float4{0.08f, 0.09f, 0.11f, 1.0f};
    // const auto labelColor = simd_float4{0.32f, 0.35f, 0.40f, 1.0f};

    // div(S::percent(1.0), S::percent(1.0), simd_float4{1.0f, 1.0f, 1.0f, 1.0f})
    //     .display(gui::Display::Flex)
    //     .flexDirection(gui::FlexDirection::Col)
    //     .flexGap(S::px(12))
    //     .padding(S::px(24))
    //     .overflow(gui::Overflow::Scroll)
    // (
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Clip").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::clip())
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis, text fits").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text("Short text").fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis, narrower than marker").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(6)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text("Wide text").fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Custom ending").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::custom(" [more]"))
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis + Scroll").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(140)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Scroll)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Nested clipping ancestor").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             div()
    //             (
    //                 text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                     .whiteSpace(gui::WhiteSpace::NoWrap)
    //             )
    //         )
    //     )
    // );


// 1st general test on min/max size
// using S = gui::Size;

// div()
//     .width(S::percent(1.0))
//     .height(S::percent(1.0))
//     .color(simd_float4{0.09,0.09,0.11,1.0})
//     .display(gui::Display::Flex)
//     .flexDirection(gui::FlexDirection::Col)
//     .flexGap(S::px(16))
//     .padding(S::px(24))
// (
//     div()
//         .width(S::percent(1.0))
//         .color(simd_float4{0.13,0.13,0.16,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Row)
//         .flexWrap(gui::FlexWrap::Wrap)
//         .flexGap(S::px(8))
//         .minHeight(S::px(60))
//         .maxHeight(S::px(120))
//         .padding(S::px(12))
//     (
//         div()
//             .height(S::px(36))
//             .color(simd_float4{0.18,0.72,0.56,1.0})
//             .flexGrow(S::px(1))
//             .minWidth(S::px(80))
//             .maxWidth(S::px(200))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .height(S::px(36))
//             .color(simd_float4{0.30,0.30,0.36,1.0})
//             .flexGrow(S::px(2))
//             .minWidth(S::px(120))
//             .maxWidth(S::px(400))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .width(S::px(60))
//             .height(S::px(36))
//             .color(simd_float4{0.22,0.22,0.28,1.0})
//             .flexShrink(S::px(0.0))
//             .cornerRadius(S::px(6))
//         ()
//     ),

//     div()
//         .width(S::percent(1.0))
//         .color(simd_float4{0.11,0.11,0.14,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::percent(0.33), S::percent(0.33), S::percent(0.33)})
//         .gridTemplateRows({S::px(80), S::px(80)})
//         .gridColumnGap(S::px(8))
//         .gridRowGap(S::px(8))
//         .minHeight(S::px(80))
//         .maxHeight(S::px(300))
//         .padding(S::px(12))
//     (
//         div()
//             .color(simd_float4{0.18,0.72,0.56,1.0})
//             .minHeight(S::px(60))
//             .maxHeight(S::px(140))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .color(simd_float4{0.30,0.30,0.36,1.0})
//             .gridColumn(2, 4)
//             .minWidth(S::px(160))
//             .maxWidth(S::px(500))
//             .minHeight(S::px(60))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .color(simd_float4{0.22,0.22,0.28,1.0})
//             .gridColumn(1, 4)
//             .minHeight(S::px(40))
//             .maxHeight(S::px(80))
//             .cornerRadius(S::px(6))
//         ()
//     ),

//     div()
//         .width(S::percent(1.0))
//         .color(simd_float4{0.13,0.13,0.16,1.0})
//         .minWidth(S::px(200))
//         .maxWidth(S::px(600))
//         .minHeight(S::px(80))
//         .maxHeight(S::px(160))
//         .padding(S::px(16))
//         .cornerRadius(S::px(8))
//     (
//         div()
//             .width(S::percent(0.6))
//             .height(S::px(20))
//             .color(simd_float4{0.18,0.72,0.56,1.0})
//             .minWidth(S::px(100))
//             .maxWidth(S::px(320))
//             .cornerRadius(S::px(4))
//         (),
//         div()
//             .width(S::percent(0.9))
//             .height(S::px(20))
//             .color(simd_float4{0.30,0.30,0.36,1.0})
//             .minWidth(S::px(140))
//             .maxWidth(S::px(480))
//             .marginTop(S::px(8))
//             .cornerRadius(S::px(4))
//         (),
//         div()
//             .width(S::percent(0.4))
//             .height(S::px(20))
//             .color(simd_float4{0.22,0.22,0.28,1.0})
//             .minWidth(S::px(80))
//             .maxWidth(S::px(200))
//             .marginTop(S::px(8))
//             .cornerRadius(S::px(4))
//         ()
//     )
// );

// using S = gui::Size;

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.09,0.09,0.11,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::px(200), S::px(200)})
    .gridColumnGap(S::px(12))
    .gridRowGap(S::px(12))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.13,0.13,0.16,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(8))
        .padding(S::px(12))
        .minWidth(S::px(120))
        .maxWidth(S::px(600))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.18,0.72,0.56,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(60))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(3))
            .minWidth(S::px(100))
            .cornerRadius(S::px(6))
            .display(gui::Display::Flex)
            .flexDirection(gui::FlexDirection::Col)
            .flexGap(S::px(6))
            .padding(S::px(8))
        (
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.18,0.72,0.56,1.0})
                .flexGrow(S::px(1))
                .minHeight(S::px(24))
                .maxHeight(S::px(60))
                .cornerRadius(S::px(4))
            (),
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.30,0.30,0.36,1.0})
                .flexGrow(S::px(2))
                .minHeight(S::px(40))
                .cornerRadius(S::px(4))
            (),
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.18,0.72,0.56,1.0})
                .flexGrow(S::px(1))
                .minHeight(S::px(24))
                .maxHeight(S::px(60))
                .cornerRadius(S::px(4))
            ()
        )
    ),

    div()
        .color(simd_float4{0.13,0.13,0.16,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(8))
        .padding(S::px(12))
    (
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(1))
            .minHeight(S::px(40))
            .maxHeight(S::px(80))
            .cornerRadius(S::px(6))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.30,0.30,0.36,1.0})
            .flexShrink(S::px(0.0))
            .height(S::px(60))
            .minWidth(S::px(80))
            .cornerRadius(S::px(6))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(2))
            .minHeight(S::px(40))
            .cornerRadius(S::px(6))
        ()
    ),

    div()
        .color(simd_float4{0.11,0.11,0.14,1.0})
        .gridColumn(1, 3)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexWrap(gui::FlexWrap::Wrap)
        .flexGap(S::px(8))
        .padding(S::px(12))
        .minHeight(S::px(80))
        .maxHeight(S::px(200))
    (
        div()
            .height(S::px(48))
            .color(simd_float4{0.18,0.72,0.56,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.30,0.30,0.36,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.18,0.72,0.56,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(240))
        .maxWidth(S::px(720))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .flexGrow(S::px(2))
            .flexShrink(S::px(1))
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(4))
            .minWidth(S::px(160))
            .maxWidth(S::px(260))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .flexGrow(S::px(3))
            .flexShrink(S::px(1))
            .minWidth(S::px(70))
            .maxWidth(S::px(140))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minHeight(S::px(180))
        .maxHeight(S::px(360))
    (
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(1))
            .minHeight(S::px(42))
            .maxHeight(S::px(80))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .flexGrow(S::px(4))
            .flexShrink(S::px(1))
            .minHeight(S::px(90))
            .maxHeight(S::px(150))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .flexGrow(S::px(2))
            .flexShrink(S::px(3))
            .minHeight(S::px(38))
            .maxHeight(S::px(90))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::percent(0.45), S::percent(0.55)})
        .gridTemplateRows({S::percent(1.0)})
        .gridColumnGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(560))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .minWidth(S::px(140))
            .maxWidth(S::px(220))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .display(gui::Display::Flex)
            .flexDirection(gui::FlexDirection::Col)
            .flexGap(S::px(8))
            .padding(S::px(8))
            .minWidth(S::px(120))
            .maxWidth(S::px(300))
            .cornerRadius(S::px(5))
        (
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.98,0.92,0.32,1.0})
                .flexGrow(S::px(2))
                .minHeight(S::px(36))
                .maxHeight(S::px(80))
                .cornerRadius(S::px(4))
            (),
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.16,0.36,0.98,1.0})
                .flexGrow(S::px(1))
                .minHeight(S::px(52))
                .maxHeight(S::px(100))
                .cornerRadius(S::px(4))
            ()
        )
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexWrap(gui::FlexWrap::Wrap)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minHeight(S::px(150))
        .maxHeight(S::px(260))
    (
        div()
            .height(S::px(52))
            .color(simd_float4{0.94,0.18,0.18,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(1))
            .minWidth(S::px(130))
            .maxWidth(S::px(210))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(52))
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .flexGrow(S::px(3))
            .flexShrink(S::px(1))
            .minWidth(S::px(90))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(52))
            .color(simd_float4{0.94,0.84,0.14,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(2))
            .minWidth(S::px(150))
            .maxWidth(S::px(240))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(52))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .flexGrow(S::px(2))
            .flexShrink(S::px(1))
            .minWidth(S::px(100))
            .maxWidth(S::px(190))
            .cornerRadius(S::px(5))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(520))
        .minHeight(S::px(150))
        .maxHeight(S::px(260))
    (
        div()
            .width(S::px(640))
            .height(S::px(56))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(500))
            .height(S::px(56))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .marginTop(S::px(10))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(760))
            .height(S::px(56))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .marginTop(S::px(10))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(220))
        .maxWidth(S::px(480))
        .minHeight(S::px(160))
        .maxHeight(S::px(260))
    (
        div()
            .width(S::percent(1.0))
            .height(S::px(90))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .minWidth(S::px(360))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .height(S::px(120))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .minWidth(S::px(520))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .height(S::px(90))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .minWidth(S::px(420))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::px(220), S::px(260), S::px(180)})
        .gridTemplateRows({S::px(110), S::px(130)})
        .gridColumnGap(S::px(10))
        .gridRowGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(560))
        .minHeight(S::px(180))
        .maxHeight(S::px(300))
    (
        div()
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .minWidth(S::px(220))
            .minHeight(S::px(110))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .minWidth(S::px(260))
            .minHeight(S::px(110))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.98,0.92,0.32,1.0})
            .minWidth(S::px(180))
            .minHeight(S::px(110))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.36,0.98,1.0})
            .minWidth(S::px(220))
            .minHeight(S::px(130))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.94,0.18,0.18,1.0})
            .minWidth(S::px(260))
            .minHeight(S::px(130))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .minWidth(S::px(180))
            .minHeight(S::px(130))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(560))
        .minHeight(S::px(180))
        .maxHeight(S::px(300))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .flexShrink(S::px(0))
            .width(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .flexShrink(S::px(0))
            .width(S::px(240))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .flexShrink(S::px(0))
            .width(S::px(300))
            .cornerRadius(S::px(5))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .justifyContent(gui::JustifyContent::SpaceBetween)
        .alignItems(gui::AlignItems::Center)
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(680))
        .minHeight(S::px(170))
    (
        div()
            .width(S::px(70))
            .height(S::px(50))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .alignSelf(gui::AlignSelf::FlexStart)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(90))
            .height(S::px(80))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(80))
            .height(S::px(55))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .alignSelf(gui::AlignSelf::FlexEnd)
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .justifyContent(gui::JustifyContent::SpaceBetween)
        .alignItems(gui::AlignItems::FlexEnd)
        .padding(S::px(12))
        .minWidth(S::px(240))
        .minHeight(S::px(180))
        .maxHeight(S::px(360))
    (
        div()
            .width(S::px(70))
            .height(S::px(44))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .alignSelf(gui::AlignSelf::FlexStart)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(150))
            .height(S::px(60))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(100))
            .height(S::px(52))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .alignSelf(gui::AlignSelf::Auto)
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
        .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
        .gridColumnGap(S::px(10))
        .gridRowGap(S::px(10))
        .alignItems(gui::AlignItems::Center)
        .padding(S::px(12))
        .minWidth(S::px(260))
        .minHeight(S::px(180))
    (
        div()
            .width(S::px(80))
            .height(S::px(46))
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .alignSelf(gui::AlignSelf::FlexStart)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(110))
            .height(S::px(70))
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(90))
            .height(S::px(52))
            .color(simd_float4{0.98,0.92,0.32,1.0})
            .alignSelf(gui::AlignSelf::FlexEnd)
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.36,0.98,1.0})
            .alignSelf(gui::AlignSelf::Stretch)
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .minHeight(S::px(44))
            .maxHeight(S::px(90))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .justifyContent(gui::JustifyContent::Center)
        .alignItems(gui::AlignItems::Stretch)
        .flexGap(S::px(12))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .minHeight(S::px(180))
    (
        div()
            .width(S::px(74))
            .color(simd_float4{0.94,0.18,0.18,1.0})
            // .alignSelf(gui::AlignSelf::Stretch)
            // .height(S::px(100))
            .minHeight(S::px(60))
            .maxHeight(S::px(150))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(120))
            .height(S::px(70))
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(90))
            .height(S::px(56))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .alignSelf(gui::AlignSelf::FlexEnd)
            .cornerRadius(S::px(5))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::fr(1.0), S::fr(1.0)})
    .gridTemplateRows({S::fr(1.0), S::fr(1.0)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(700))
        .minHeight(S::px(170))
    (
        div()
            .width(S::percent(0.35))
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(140))
            .height(S::percent(1.0))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .minWidth(S::px(100))
            .maxWidth(S::px(220))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(260))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(240))
        .minHeight(S::px(180))
        .maxHeight(S::px(360))
    (
        div()
            .width(S::percent(1.0))
            .height(S::percent(0.25))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .minHeight(S::px(42))
            .maxHeight(S::px(90))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(180))
            .height(S::px(64))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .minWidth(S::px(120))
            .maxWidth(S::px(260))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(0.75))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .flexGrow(S::px(1))
            .minHeight(S::px(50))
            .maxHeight(S::px(130))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::px(140), S::percent(0.5), S::fr(1.0)})
        .gridTemplateRows({S::px(70), S::percent(0.45), S::fr(1.0)})
        .gridColumnGap(S::px(10))
        .gridRowGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(300))
        .minHeight(S::px(190))
    (
        div()
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .minWidth(S::px(90))
            .minHeight(S::px(46))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .minWidth(S::px(130))
            .maxWidth(S::px(220))
            .minHeight(S::px(50))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.98,0.92,0.32,1.0})
            .minWidth(S::px(80))
            .maxWidth(S::px(180))
            .minHeight(S::px(50))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.36,0.98,1.0})
            .gridColumn(1, 3)
            .minHeight(S::px(70))
            .maxHeight(S::px(120))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.94,0.18,0.18,1.0})
            .minWidth(S::px(90))
            .minHeight(S::px(60))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexWrap(gui::FlexWrap::Wrap)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .minHeight(S::px(180))
        .maxHeight(S::px(320))
    (
        div()
            .width(S::percent(0.4))
            .height(S::px(54))
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .minWidth(S::px(110))
            .maxWidth(S::px(220))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(54))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(150))
            .height(S::px(54))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .minWidth(S::px(120))
            .maxWidth(S::px(210))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(0.65))
            .height(S::px(54))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .minWidth(S::px(160))
            .maxWidth(S::px(300))
            .cornerRadius(S::px(5))
        ()
    )
);
*/


// complex test scene
// div()
//     .width(S::percent(1.0))
//     .height(S::percent(1.0))
//     .color(simd_float4{0.04,0.04,0.05,1.0})
//     .display(gui::Display::Grid)
//     .gridTemplateColumns({S::px(260), S::percent(0.35), S::fr(1.0)})
//     .gridTemplateRows({S::px(178), S::fr(1.0), S::px(148)})
//     .gridColumnGap(S::px(14))
//     .gridRowGap(S::px(14))
//     .padding(S::px(22))
// (
//     div()
//         .gridColumn(1, 4)
//         .gridRow(1, 2)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Row)
//         .flexGap(S::px(12))
//         .padding(S::px(12))
//         .minHeight(S::px(150))
//     (
//         div().width(S::px(170)).height(S::percent(1.0)).minWidth(S::px(120)).maxWidth(S::px(220)).color(simd_float4{0.00,0.78,0.58,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.28)).height(S::percent(1.0)).minWidth(S::px(180)).maxWidth(S::px(420)).color(simd_float4{0.96,0.22,0.62,1.0}).cornerRadius(S::px(5))(),
//         div().height(S::percent(1.0)).flexGrow(S::px(1)).minWidth(S::px(180)).maxWidth(S::px(520)).color(simd_float4{0.98,0.92,0.32,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::px(140)).height(S::percent(1.0)).minWidth(S::px(110)).maxWidth(S::px(180)).color(simd_float4{0.16,0.84,0.90,1.0}).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(1, 2)
//         .gridRow(2, 3)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(S::px(10))
//         .padding(S::px(12))
//         .minWidth(S::px(220))
//         .minHeight(S::px(220))
//     (
//         div().width(S::percent(1.0)).height(S::percent(0.22)).minHeight(S::px(46)).maxHeight(S::px(90)).color(simd_float4{0.95,0.18,0.22,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::px(155)).height(S::px(62)).minWidth(S::px(120)).maxWidth(S::px(210)).color(simd_float4{0.10,0.72,0.95,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.72)).flexGrow(S::px(1)).minHeight(S::px(70)).maxHeight(S::px(180)).color(simd_float4{1.00,0.48,0.12,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.45)).height(S::px(48)).minWidth(S::px(90)).maxWidth(S::px(150)).color(simd_float4{0.54,0.28,0.98,1.0}).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(2, 3)
//         .gridRow(2, 3)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::px(120), S::percent(0.45), S::fr(1.0)})
//         .gridTemplateRows({S::px(64), S::percent(0.5), S::fr(1.0)})
//         .gridColumnGap(S::px(10))
//         .gridRowGap(S::px(10))
//         .padding(S::px(12))
//         .minWidth(S::px(300))
//         .minHeight(S::px(220))
//     (
//         div().color(simd_float4{0.32,0.95,0.42,1.0}).minWidth(S::px(90)).minHeight(S::px(46)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.68,0.28,0.96,1.0}).minWidth(S::px(130)).maxWidth(S::px(240)).minHeight(S::px(50)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.98,0.92,0.32,1.0}).minWidth(S::px(80)).maxWidth(S::px(170)).minHeight(S::px(50)).cornerRadius(S::px(5))(),
//         div().gridColumn(1, 3).color(simd_float4{0.16,0.36,0.98,1.0}).minHeight(S::px(74)).maxHeight(S::px(130)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.94,0.18,0.18,1.0}).minWidth(S::px(90)).minHeight(S::px(60)).cornerRadius(S::px(5))(),
//         div().gridColumn(1, 4).color(simd_float4{0.16,0.84,0.90,1.0}).minHeight(S::px(46)).maxHeight(S::px(80)).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(3, 4)
//         .gridRow(2, 3)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Row)
//         .flexWrap(gui::FlexWrap::Wrap)
//         .flexGap(S::px(10))
//         .padding(S::px(12))
//         .minWidth(S::px(260))
//         .minHeight(S::px(220))
//     (
//         div().width(S::percent(0.36)).height(S::px(58)).minWidth(S::px(110)).maxWidth(S::px(210)).color(simd_float4{0.95,0.84,0.16,1.0}).cornerRadius(S::px(5))(),
//         div().height(S::px(58)).flexGrow(S::px(1)).minWidth(S::px(120)).maxWidth(S::px(240)).color(simd_float4{0.00,0.78,0.58,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::px(180)).height(S::px(58)).minWidth(S::px(140)).maxWidth(S::px(220)).color(simd_float4{0.96,0.22,0.62,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.62)).height(S::px(58)).minWidth(S::px(180)).maxWidth(S::px(340)).color(simd_float4{0.16,0.36,0.98,1.0}).cornerRadius(S::px(5))(),
//         div().height(S::px(58)).flexGrow(S::px(1)).minWidth(S::px(90)).maxWidth(S::px(160)).color(simd_float4{1.00,0.48,0.12,1.0}).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(1, 4)
//         .gridRow(3, 4)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::percent(0.25), S::px(220), S::fr(1.0), S::px(160)})
//         .gridTemplateRows({S::fr(1.0)})
//         .gridColumnGap(S::px(12))
//         .padding(S::px(12))
//         .minHeight(S::px(120))
//     (
//         div().color(simd_float4{0.68,0.28,0.96,1.0}).minWidth(S::px(120)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.10,0.72,0.95,1.0}).minWidth(S::px(160)).maxWidth(S::px(220)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.32,0.95,0.42,1.0}).minWidth(S::px(220)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.95,0.18,0.22,1.0}).minWidth(S::px(120)).cornerRadius(S::px(5))()
//     )
// );
}
