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



    div(NewArch::Size::percent(1.0), NewArch::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
        .position(NewArch::Relative)
    (
        div(NewArch::Size::percent(1.0), NewArch::Size::px(100), simd_float4{0.9,0.9,0.9,1.0})
            .display(NewArch::Flex)
        (
            div(NewArch::Size::px(100), NewArch::Size::px(80), simd_float4{1.0,0.3,0.3,1.0}),
            div()
            .width(NewArch::Size::px(80))
            // .height(NewArch::Size::px(80))
            .color(simd_float4{0.3,1.0,0.3,1.0})
            
            .flexGrow(NewArch::Size::px(1))
            (
                text("hello world fsdfsdfsdfsdfsdfsdf")
            ),
            div(NewArch::Size::px(100), NewArch::Size::px(80), simd_float4{0.3,0.3,1.0,1.0})
        )
    );
}

