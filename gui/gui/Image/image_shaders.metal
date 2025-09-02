//
//  image_shaders.metal
//  gui
//
//  Created by Taanish Reja on 8/28/25.
//

#include <metal_stdlib>
#include "../common.metal"

using namespace metal;

struct ImageUniforms {
    float2 rectCenter;
    float2 halfExtent;
    float cornerRadius;
    float borderWidth;
    float4 borderColor;
};

struct ImageVertexIn {
    float2 position [[attribute(0)]];
    float2 texCords [[attribute(1)]];
};

struct ImageVertexOut {
    float4 position [[position]];
    float4 worldPosition;
    float2 texCords;
};

vertex ImageVertexOut vertex_image(
   ImageVertexIn in [[stage_in]],
   constant FrameInfo* frameInfo [[buffer(1)]]
)
{
    ImageVertexOut out;
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
    
    float2 localPosition = in.worldPosition.xy - uniforms->rectCenter;
    float d = rounded_rect_sdf(localPosition, uniforms->halfExtent, uniforms->cornerRadius);

    
    float px = fwidth(d);

    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);
    
    float innerD = d + uniforms->borderWidth;
    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);
    
    float borderMask = outerMask - innerMask;
    float fillMask = innerMask;
    
    float4 fillColor = color;
    float4 borderColor = uniforms->borderColor;
    
    float3 premulFill = fillColor.rgb * fillColor.a * fillMask;
    float3 premulBorder = borderColor.rgb * borderColor.a * borderMask;
    
    float alpha = borderMask * borderColor.a + fillMask * fillColor.a;

    float3 rgb = premulFill + premulBorder;
    
    if (alpha > 1e-6)
        rgb /= alpha;
    
    return float4(rgb, alpha);

}
