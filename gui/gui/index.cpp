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


    // Test: Fixed vs Absolute positioning
    // - Gray container at (100, 100) with position: relative
    // - Purple box: absolute, bottom: 10 -> should be 10px from bottom of gray container
    // - Red box: fixed, bottom: 10 -> should be 10px from bottom of viewport
    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    (
        div(NewArch::Size::px(300), NewArch::Size::px(200), simd_float4{0.7,0.7,0.7,1.0})
            .position(NewArch::Relative)
            .marginTop(100)
            .marginLeft(100)
        (
            div(NewArch::Size::px(50), NewArch::Size::px(50), simd_float4{0.5,0.0,1.0,1.0})
                .position(NewArch::Absolute)
                .bottom(NewArch::Size::px(10))
                .left(NewArch::Size::px(10))
            (),
            div(NewArch::Size::px(50), NewArch::Size::px(50), simd_float4{1.0,0.0,0.0,1.0})
                .position(NewArch::Fixed)
                .bottom(NewArch::Size::px(10))
                .right(NewArch::Size::px(10))
            ()
        )
    );
}

