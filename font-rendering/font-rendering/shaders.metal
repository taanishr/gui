//
//  shaders.metal
//  font-rendeirng
//
//  Created by Taanish Reja on 7/23/25.
//

#include <metal_stdlib>
using namespace metal;

struct Constants {
    unsigned long nPoints;
    unsigned long numContours;
    float scaling;
};

struct ContourBounds {
    unsigned long start;
    unsigned long end;
};

struct VertexIn {
    float2 position [[attribute(0)]];

};


struct VertexOut {
    float4 position [[position]];
    float2 worldPos;
};

vertex VertexOut vertex_main(
     VertexIn in [[stage_in]],
     constant Constants& constants [[buffer(1)]]
)
{
    VertexOut out;
    float2 scaledPos = in.position / constants.scaling;
    out.position = float4(scaledPos, 0.0, 1.0);
    out.worldPos = scaledPos;
    
    return out;
}

fragment float4 fragment_main(
    VertexOut in [[stage_in]],
    constant float2* vertices [[buffer(1)]],
    constant ContourBounds* contourBounds [[buffer(2)]],
    constant Constants& constants [[buffer(3)]]
)
{
    float x = in.worldPos.x;
    float y = in.worldPos.y;
    
    
    int intersections = 0;
    
    
    for (unsigned long c = 0; c < constants.numContours; ++c) {
        ContourBounds cb = contourBounds[c];

        unsigned long offset = cb.start;
        unsigned long contourSize = cb.end-cb.start;
        
        for (unsigned long p = 0; p < contourSize; ++p) {
            unsigned long edgeStart = offset + p;
            unsigned long edgeEnd = offset + (p+1)%contourSize;
            
            float2 v1 = vertices[edgeStart] / constants.scaling;
            float2 v2 = vertices[edgeEnd] / constants.scaling;
            
            if (v1.y > y != v2.y > y) {
                float intersectX = v1.x + (y - v1.y) * ((v2.x - v1.x) / (v2.y - v1.y));
                if (intersectX > x) {
                    ++intersections;
                }
            }
            
            
        }
    }
   
    if (intersections % 2 == 0)
        return float4(1,1,1,1);
    else
        return float4(0,0,0,1);
}
