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

        drawable->color = simd_float4{0,0.5,0,0.5};
    };
    
    return div().h(100.0).w(100.0).cornerRadius(50.0).color(RGB{0,0,128,0.5}).on<EventType::Click>(onClick).flex()
        (
            div().h(100.0).w(100.0).x(40).y(128.0).color(RGB{128,0,0,0.5})(
               text("").fontSize(24.0).x(10.0).y(25.0).on<EventType::KeyboardDown>(onInput),
                div().h(50.0).w(50.0).x(128.0).cornerRadius(25).color(RGB{255,255,255,1}).borderColor(RGB{125,0,0,0.5}).borderWidth(2)
            ),
            image("/Users/treja/build_journal/Screenshot 2025-07-19 at 8.06.55 PM.png").x(150).y(50).w(275).h(100).cornerRadius(16).borderWidth(2).borderColor(RGB(50,0,126,1))
       );
    
    
}
