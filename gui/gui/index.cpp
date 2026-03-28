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

    using S = NewArch::Size;

    // Grid demo: holy grail layout
    div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
        .display(NewArch::Display::Grid)
        .gridTemplateColumns({S::fr(1), S::fr(2), S::fr(1)})
        .gridTemplateRows({S::px(60), S::fr(1), S::px(40)})
        .gridColumnGap(S::px(8))
        .gridRowGap(S::px(8))
        .padding(S::px(8))
    (
        // Header — spans all 3 columns
        div().gridColumn(1, 4).gridRow(1, 2)
            .color(simd_float4{0.2,0.4,0.8,1.0})
            .cornerRadius(S::px(8))
            .display(NewArch::Display::Flex)
            .alignItems(NewArch::AlignItems::Center)
            .justifyContent(NewArch::JustifyContent::Center)
        (
            text("Header").fontSize(S::pt(20)).color(simd_float4{1,1,1,1})
        ),

        // Left sidebar
        div().gridColumn(1, 2).gridRow(2, 3)
            .color(simd_float4{0.9,0.9,0.95,1.0})
            .cornerRadius(S::px(8))
            .padding(S::px(12))
        (
            text("Sidebar").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
        ),

        // Main content
        div().gridColumn(2, 3).gridRow(2, 3)
            .color(simd_float4{1.0,1.0,1.0,1.0})
            .cornerRadius(S::px(8))
            .padding(S::px(12))
        (
            text("Main Content").fontSize(S::pt(14)).color(simd_float4{0.1,0.1,0.1,1})
        ),

        // Right sidebar
        div().gridColumn(3, 4).gridRow(2, 3)
            .color(simd_float4{0.9,0.9,0.95,1.0})
            .cornerRadius(S::px(8))
            .padding(S::px(12))
        (
            text("Panel").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
        ),

        // Footer — spans all 3 columns
        div().gridColumn(1, 4).gridRow(3, 4)
            .color(simd_float4{0.3,0.3,0.35,1.0})
            .cornerRadius(S::px(8))
            .display(NewArch::Display::Flex)
            .alignItems(NewArch::AlignItems::Center)
            .justifyContent(NewArch::JustifyContent::Center)
        (
            text("Footer").fontSize(S::pt(14)).color(simd_float4{1,1,1,1})
        )
    );
}

