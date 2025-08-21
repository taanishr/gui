//
//  shell_shaders.metal
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#include <metal_stdlib>
#include "../common.metal"

using namespace metal;

struct ShellVertexIn {
    float2 position [[attribute(0)]];
};

struct ShellVertexOut {
    float4 position [[position]];
};

struct ShellUniforms {
    float4 color;
};

vertex ShellVertexOut vertex_shell(
   ShellVertexIn in [[stage_in]],
   constant FrameInfo* frameInfo [[buffer(1)]]
)
{
    ShellVertexOut out;
    float2 adjustedPosition = toNDC(in.position, frameInfo->width, frameInfo->height);
    out.position = float4(adjustedPosition, 0.0, 1.0);
    return out;
}

fragment float4 fragment_shell(
    ShellVertexOut in [[stage_in]],
    constant ShellUniforms* uniforms [[buffer(0)]]
)
{
    float4 color = uniforms->color;
    return color;
}
