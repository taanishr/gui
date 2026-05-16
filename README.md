## WIP C++ gpu-rendered GUI library with aims of a supporting a wide range of elements and styling attributes and an ergonomic api.

## Layout

- `src/`: C++ sources, headers, and Metal shaders.
- `assets/`: checked-in sample/runtime assets.
- `apple-extensions/`: Swift AppKit and MetalKit extension projects.
- `build/apple-extensions/`: ignored Swift extension build products linked by the main app.
- `scripts/`: local build/run helpers.

*Example:*
```
static int count = 0;

auto index() -> void {

    auto onClick = [](auto& desc, const Event& event){
        count += 1;

        if (count % 2 == 0) {
            desc.color = simd_float4{1.0,0.0,0.0,1.0};
        }else {
            desc.color = simd_float4{0.0,0.0,1.0,1.0};
        }

        std::println("hello world {}", count);
    };

    div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
        .borderColor(simd_float4{0.77,0.71,1.0,1.0})
        .borderWidth(1.0)
    (
         div(gui::Size::percent(0.2), gui::Size::percent(1.0), simd_float4{1.0,0.5,1.0,0.8})(
            div(gui::Size::px(60), gui::Size::px(30), simd_float4{0.498,0.0,1.0,1.0})
                .marginTop(30)
                .marginLeft(gui::Size::autoSize())
                .marginRight(gui::Size::autoSize())
                .cornerRadius(7.5)
                .paddingLeft(9.0)
                .paddingTop(4.5)
                .borderColor(simd_float4{0.77,0.71,1.0,1.0})
                .borderWidth(1.0)
                .addEventListener(EventType::MouseDown, onClick)
            (
                text("Startfsd")
                    .fontSize(48.0)
                    .font(Arial)
                    .marginLeft(10)
                    .marginRight(10)
                    .color(simd_float4{0,0,0,1})
                    ,
                text("fsdfsdfsda   dads sdsfsdsds")
                    .fontSize(48.0)
                    .font(Arial)
                    .color(simd_float4{0,0,0,1})
            ),
            div(
                gui::Size::px(60), gui::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
            ).marginTop(10)
         )
    );
}
```
