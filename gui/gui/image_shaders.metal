//
//  image_shaders.metal
//  gui
//
//  Created by Taanish Reja on 8/28/25.
//

#include <metal_stdlib>
#include "common.metal"

using namespace metal;

struct ImageStyleUniforms {
    float cornerRadius;
    float borderWidth;
    float4 borderColor;
};

struct ImageGeometryUniforms {
    float2 rectCenter;
    float2 halfExtent;
};

struct ImageUniforms {
    ImageStyleUniforms style;
    ImageGeometryUniforms geometry;
};

struct ImageVertexIn {
    float2 position [[attribute(0)]];
    float2 texCords [[attribute(1)]];
    uint   atom_id  [[attribute(2)]];
};

struct ImageVertexOut {
    float4 position [[position]];
    float4 worldPosition;
    float2 texCords;
};

vertex ImageVertexOut vertex_image(
    ImageVertexIn in [[stage_in]],
    constant float2* offsets [[buffer(1)]],
    constant FrameInfo* frameInfo [[buffer(2)]]
)
{
    ImageVertexOut out;

    in.position += offsets[in.atom_id];

    float2 adjustedPosition = toNDC(in.position, frameInfo->width, frameInfo->height);
    out.position = float4(adjustedPosition, 0.0, 1.0);
    out.worldPosition = float4(in.position, 0.0, 1.0);
    out.texCords = in.texCords;

    return out;
}
fragment float4 fragment_image(
    ImageVertexOut in [[stage_in]],
    constant ImageUniforms* uniforms[[buffer(0)]],
    texture2d<float, access::sample> textureMap [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 color = textureMap.sample(textureSampler, in.texCords);
    
    float2 localPosition = in.worldPosition.xy - uniforms->geometry.rectCenter;
    float d = rounded_rect_sdf(localPosition, uniforms->geometry.halfExtent, uniforms->style.cornerRadius);

    
    float px = fwidth(d);

    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);
    
    float innerD = d + uniforms->style.borderWidth;
    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);
    
    float borderMask = outerMask - innerMask;
    float fillMask = innerMask;
    
    float4 fillColor = color;
    float4 borderColor = uniforms->style.borderColor;
    
    float3 premulFill = fillColor.rgb * fillColor.a * fillMask;
    float3 premulBorder = borderColor.rgb * borderColor.a * borderMask;
    
    float alpha = borderMask * borderColor.a + fillMask * fillColor.a;

    float3 rgb = premulFill + premulBorder;
    
    if (alpha > 1e-6)
        rgb /= alpha;
    
    return float4(rgb, alpha);
}
