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
    // // turns out they just were not being downsampled

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
    //         .display(NewArch::Display::Flex)
    //         .alignItems(NewArch::AlignItems::Center)
    //         .justifyContent(NewArch::JustifyContent::Center)
    //     (
    //         text("Footer").fontSize(S::pt(14)).color(simd_float4{1,1,1,1})
    //     )
    // );

    // div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .display(NewArch::Display::Flex)
    //     .flexDirection(NewArch::FlexDirection::Col)
    //     .padding(NewArch::Size::px(32))
    // (
    //     div()
    //         .display(NewArch::Display::Flex)
    //         .flexDirection(NewArch::FlexDirection::Col)
    //         .color(simd_float4{0.5,0.5,0.5,1.0})
    //     (
    //         div()
    //             .color(simd_float4{1.0,1.0,1.0,1.0})
    //             .display(NewArch::Display::Flex)
    //         (
    //             text("Flight").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
    //             text("UA 837").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
    //         )
    //     )
    // );

   
    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    .display(NewArch::Display::Flex)
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
                text("=").fontSize(NewArch::Size::pt(18)).color(simd_float4{0.5,0.5,0.5,1.0})
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
                text("You: San Francisco").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
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
                text("Import").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
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
                text("Add trip").fontSize(NewArch::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
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
            .width(NewArch::Size::px(200.0))
            .color(simd_float4{1.0,1.0,1.0,1.0})
            .borderColor(simd_float4{0.88,0.88,0.88,1.0})
            .borderWidth(NewArch::Size::px(1.0))
            .display(NewArch::Display::Flex)
            .flexDirection(NewArch::FlexDirection::Col)
            .flexGap(NewArch::Size::px(4))
            .padding(NewArch::Size::px(12))
        (
            text("TRIPS").fontSize(NewArch::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
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
                text("Tokyo & Kyoto").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text("Mar 10 - Mar 24").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                text("NYC Weekend").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text("Apr 4 - Apr 7").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                text("London + Paris").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text("Jun 1 - Jun 12").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                    text("Tokyo & Kyoto").fontSize(NewArch::Size::pt(20)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text("Mar 10 - Mar 24 · 14 days").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
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
                    text("Edit").fontSize(NewArch::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
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
                    text("SFO -> NRT").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                    ,text("Mar 10 · United 837 · 11h 30m").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                    text("[Flight]").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.4,0.3,1.0,1.0})
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
                    text("Park Hyatt Tokyo").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text("Mar 11 - Mar 17 · 6 nights").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                    text("[Hotel]").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.2,0.6,0.9,1.0})
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
                    text("Tokyo -> Kyoto").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
                    text("Mar 17 · Shinkansen · 2h 15m").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                    text("[Train]").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.1,0.7,0.4,1.0})
                )
            )
        )
        // Detail panel
        ,div()
            .width(NewArch::Size::px(280.0))
            .color(simd_float4{1.0,1.0,1.0,1.0})
            .borderColor(simd_float4{0.88,0.88,0.88,1.0})
            .borderWidth(NewArch::Size::px(1.0))
            .display(NewArch::Display::Flex)
            .flexDirection(NewArch::FlexDirection::Col)
            .flexGap(NewArch::Size::px(16))
            .padding(NewArch::Size::px(16))
        (
            div()
                .color(simd_float4{0.0,0.0,0.0,0.0})
                .display(NewArch::Display::Flex)
                .alignItems(NewArch::AlignItems::Center)
                .justifyContent(NewArch::JustifyContent::SpaceBetween)
            (
                text("Flight Details").fontSize(NewArch::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
                text("x").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0})
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
                    text("Flight").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text("UA 837").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text("Departs").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text("10:45 AM").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text("Arrives").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text("3:15 PM +1").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text("Duration").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text("11h 30m").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text("Seat").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text("42A").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
                ),
                div()
                    .color(simd_float4{0.0,0.0,0.0,0.0})
                    .display(NewArch::Display::Flex)
                    .justifyContent(NewArch::JustifyContent::SpaceBetween)
                (
                    text("Class").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
                    text("Economy").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
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
                text("Local time at destination").fontSize(NewArch::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
                text("3:15 PM JST (UTC+9)").fontSize(NewArch::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
            )
        )
    )
);

}

