//
//  shaders.metal
//  font-rendeirng
//
//  Created by Taanish Reja on 7/23/25.
//

#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float2 position [[attribute(0)]];
};

vertex float4 vertex_main(
    VertexIn in [[stage_in]]
)
{
    return float4(in.position, 0.0, 1.0);
}

fragment float4 fragment_main(
                              VertexIn in [[stage_in]]
)
{
    return float4(0,0,0,1);
}
