//
//  image_shaders.metal
//  gui
//
//  Created by Taanish Reja on 8/28/25.
//

#include <metal_stdlib>
#include "../common.metal"

using namespace metal;

struct ImageVertexIn {
    float2 position [[attribute(0)]];
    float2 texCords [[attribute(1)]];
};

struct ImageVertexOut {
    float4 position [[position]];
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
    out.texCords = in.texCords;
    return out;
}

fragment float4 fragment_image(
    ImageVertexOut in [[stage_in]],
    texture2d<float, access::sample> textureMap [[texture(0)]],
    sampler textureSampler [[sampler(0)]]
) {
    float4 color = textureMap.sample(textureSampler, in.texCords);
    return color;
}
