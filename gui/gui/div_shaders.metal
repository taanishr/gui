//
//  shell_shaders.metal
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#include <metal_stdlib>
#include "common.metal"

using namespace metal;

struct DivVertexIn {
    float2 position [[attribute(0)]];
    unsigned int atom_id [[attribute(1)]];
};

struct DivVertexOut {
    float4 position [[position]];
    float4 worldPosition;
};

//struct ShellUniforms {
//    simd_float4 color;
//    float cornerRadius;
//    float borderWidth;
//    simd_float4 borderColor;
//    simd_float2 rectCenter;
//    simd_float2 halfExtent; 
//};

struct DivStyleUniforms {
    simd_float4 color;
    float cornerRadius;
    float borderWidth;
    simd_float4 borderColor;
};

struct DivGeometryUniforms {
    simd_float2 rectCenter;
    simd_float2 halfExtent;
};

struct DivUniforms {
    DivStyleUniforms style;
    DivGeometryUniforms geometry;
};

//vertex ShellVertexOut vertex_shell(
//   ShellVertexIn in [[stage_in]],
//   constant FrameInfo* frameInfo [[buffer(1)]]
//)
//{
//    ShellVertexOut out;
//    float2 adjustedPosition = toNDC(in.position, frameInfo->width, frameInfo->height);
//    out.position = float4(adjustedPosition, 0.0, 1.0);
//    out.worldPosition = float4(in.position, 0.0, 1.0);
//    return out;
//}
//
//// outer color
//
//// inner color?
//
//fragment float4 fragment_shell(
//    ShellVertexOut in [[stage_in]],
//    constant ShellUniforms* uniforms [[buffer(0)]]
//)
//{
//    float2 localPosition = in.worldPosition.xy - uniforms->rectCenter;
//    float d = rounded_rect_sdf(localPosition, uniforms->halfExtent, uniforms->cornerRadius);
//
//    
//    float px = fwidth(d);
//
//    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);
//    
//    float innerD = d + uniforms->borderWidth;
//    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);
//    
//    float borderMask = outerMask - innerMask;
//    float fillMask = innerMask;
//    
//    float4 fillColor = uniforms->color;
//    float4 borderColor = uniforms->borderColor;
//    
//    float3 premulFill = fillColor.rgb * fillColor.a * fillMask;
//    float3 premulBorder = borderColor.rgb * borderColor.a * borderMask;
//    
//    float alpha = borderMask * borderColor.a + fillMask * fillColor.a;
//
//    float3 rgb = premulFill + premulBorder;
//    
//    if (alpha > 1e-6)
//        rgb /= alpha;
//    
//    return float4(rgb, alpha);
//}

vertex DivVertexOut vertex_shell(
   DivVertexIn in [[stage_in]],
   constant float2* offsets [[buffer(1)]],
   constant FrameInfo* frameInfo [[buffer(2)]]
)
{
    DivVertexOut out;
    
    // todo: add the offsets
    in.position += offsets[in.atom_id];
    
    float2 adjustedPosition = toNDC(in.position, frameInfo->width, frameInfo->height);
    out.position = float4(adjustedPosition, 0.0, 1.0);
    out.worldPosition = float4(in.position, 0.0, 1.0);
    return out;
}

fragment float4 fragment_shell(
    DivVertexOut in [[stage_in]],
    constant DivUniforms* uniforms [[buffer(0)]]
)
{
    // todo: figure out uniforms cpu side
    float2 localPosition = in.worldPosition.xy - uniforms->geometry.rectCenter;
    float d = rounded_rect_sdf(localPosition, uniforms->geometry.halfExtent, uniforms->style.cornerRadius);

    float px = fwidth(d);

    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);
    
    float innerD = d + uniforms->style.borderWidth;
    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);
    
    float borderMask = outerMask - innerMask;
    float fillMask = innerMask;
    
    float4 fillColor = uniforms->style.color;
    float4 borderColor = uniforms->style.borderColor;
    
    float3 premulFill = fillColor.rgb * fillColor.a * fillMask;
    float3 premulBorder = borderColor.rgb * borderColor.a * borderMask;
    
    float alpha = borderMask * borderColor.a + fillMask * fillColor.a;

    float3 rgb = premulFill + premulBorder;
    
    if (alpha > 1e-6)
        rgb /= alpha;
    
    return float4(rgb, alpha);
}
