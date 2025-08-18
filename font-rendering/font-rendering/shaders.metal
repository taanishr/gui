//
//  shaders.metal
//  font-rendeirng
//
//  Created by Taanish Reja on 7/23/25.
//

#include <metal_stdlib>
using namespace metal;

struct TextUniforms {
    float3 color;
    unsigned long numContours;
};

struct ContourMeta {
    unsigned long start;
    unsigned long end;
    float minX;
    float maxX;
    float minY;
    float maxY;
};

struct VertexIn {
    float2 position [[attribute(0)]];
};


struct VertexOut {
    float4 position [[position]];
    float2 worldPos;
};

vertex VertexOut vertex_main(
     VertexIn in [[stage_in]]
)
{
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.worldPos = in.position;
    
    return out;
}

// my attempt
// min distance to segment?
// check intersections (want to see if its outside)
// open questions: why do we need the fwidth derivatives?
    // fragment shaders calculated in blocks; screenspace derivatives on the distance can help us find the size of a pixel based on the derivative with pixels around it
// why do we need the sign?


inline float signed_dist(float2 p, float2 a, float2 b) {
    float2 ba = b - a;
    float2 pa = p - a;
    
    float2 proj = ba*clamp(dot(pa, ba)/dot(ba,ba), 0.0, 1.0); // get the projection of the distance vector
        
    return distance_squared(pa, proj); // get the distance between the vector pa and the projection
}

fragment float4 fragment_main(
    VertexOut in [[stage_in]],
    constant float2* vertices [[buffer(1)]],
    constant ContourMeta* contourBounds [[buffer(2)]],
    constant TextUniforms& uniforms [[buffer(3)]]
)
{
    float2 point = in.worldPos.xy;
    
    
    int intersections = 0;
    float minDist = 1e20;
    
    
    
    for (unsigned long c = 0; c < uniforms.numContours; ++c) {
        ContourMeta cb = contourBounds[c];

        
//        if (point.x < cb.minX || point.x >= cb.maxX || point.y < cb.minY || point.y >= cb.maxY)
//            continue;

        unsigned long offset = cb.start;
        unsigned long contourSize = cb.end-cb.start;
        
        for (unsigned long p = 0; p < contourSize; ++p) {
            unsigned long edgeStart = offset + p;
            unsigned long edgeEnd = offset + (p+1)%contourSize;
            
            float2 v1 = vertices[edgeStart];
            float2 v2 = vertices[edgeEnd];
            
            if (v1.y > point.y != v2.y > point.y) {
                float intersectX = v1.x + (point.y - v1.y) * ((v2.x - v1.x) / (v2.y - v1.y));
                if (intersectX > point.x) {
                    ++intersections;
                }
            }
            
            
//            float sd = signed_dist(point, v1, v2);
//            minDist = min(minDist, sd);
        }
    }
    
    bool inside = intersections & 1;
    
    float sd = inside ? -minDist : minDist;
    
    float px = fwidth(sd);
    
    float alpha = clamp(0.5 - sd/px, 0.0, 1.0);

    float3 rgb = mix(uniforms.color, uniforms.color, alpha);
    
    return float4(rgb*alpha,alpha);
}
