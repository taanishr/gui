// all your code goes here

#include "index.hpp"

auto index() -> Element
{
    auto onInput = [](auto& self, const auto& payload){
        auto drawable = self.drawable.get();

        if (payload.ch == '\x7F') {
            drawable->removeChar();
        }else {
            drawable->addChar(payload.ch);
        }
    };

    auto onClick = [](auto& self, const auto& payload) {
        auto drawable = self.drawable.get();

        drawable->color = simd_float4{0,0.5,0,1};
    };
    
//    return
//        div()(
//            div().h(50).color(RGB{0,128,128,1}).relativePos().x(60).y(20),
//            div().h(100).cornerRadius(50.0).color(RGB{0,0,128,0.5}),
//            div().h(60).w(60).cornerRadius(30).color(RGB{255,255,255,1}).borderWidth(2).borderColor(RGB({256,0,0,1})).absolutePos().x(200).y(200)
//        );

    
    return div().h(512).w(512).color(RGB{240,240,240,1}) (
        div().h(400).w(400).x(20).y(50).color(RGB{144,238,144,1}).relativePos()(
            text("Inline element 1 ").fontSize(16.0).color(RGB{0,0,0,1})(
                text("Nested inline element 1 ").fontSize(16.0).color(RGB{0,0,0,1}),
                text("Nested inline element 2").fontSize(16.0).color(RGB{0,0,0,1}),
                div().h(10).w(60).color(RGB{28,51,16,1})

            ),
            text("Inline element 2").fontSize(16.0).color(RGB{0,0,0,1}),
            div().h(60).w(60).cornerRadius(30)
                .color(RGB{0,0,255,1})
                .absolutePos().x(50).y(100)
        ),

        div().h(60).w(60).cornerRadius(30)
            .color(RGB{255,0,0,1})
            .fixedPos().x(432).y(432)
    );

    
//    return div().h(512).w(512).color(RGB{220,220,220,1}) ( // outer window (gray)
//         // Container with relative positioning
//         div().h(200).w(200).x(20).y(20).color(RGB{144,238,144,1}).relativePos() ( // green box
//             // Absolute element pinned inside container
//             div().h(50).w(50).cornerRadius(25)
//                 .color(RGB{0,0,255,1}) // blue circle
//                 .absolutePos().x(30).y(30).on<EventType::Click>(onClick)
//         ),
//
//         // Fixed element pinned relative to viewport
//         div().h(50).w(50).cornerRadius(25)
//             .color(RGB{255,0,0,1}) // red circle
//             .borderWidth(2).borderColor(RGB{0,0,0,1})
//             .fixedPos().x(400).y(400),
//          text("").fontSize(24.0).x(10.0).y(25.0).on<EventType::KeyboardDown>(onInput)
//     );
//    return div().h(100.0).w(100.0).cornerRadius(50.0).color(RGB{0,0,128,0.5}).on<EventType::Click>(onClick)
//        (
//            div().h(100.0).w(100.0).x(40).y(128.0).color(RGB{128,0,0,0.5})(
//               text("").fontSize(24.0).x(10.0).y(25.0).on<EventType::KeyboardDown>(onInput),
//                div().h(50.0).w(50.0).x(128.0).cornerRadius(25).color(RGB{255,255,255,1}).borderColor(RGB{125,0,0,0.5}).borderWidth(2)
//            ),
//            image("/Users/treja/build_journal/Screenshot 2025-07-19 at 8.06.55 PM.png").x(150).y(50).w(275).h(100).cornerRadius(16).borderWidth(2).borderColor(RGB(50,0,126,1))
//       );
    
    
}
