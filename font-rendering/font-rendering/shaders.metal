//
//  shaders.metal
//  font-rendeirng
//
//  Created by Taanish Reja on 7/23/25.
//

#include <metal_stdlib>
using namespace metal;

float2 toNDC(const float2 pt, float width = 512.0f, float height = 512.0f) {
    float ndcX = (pt.x / width) * 2.0f - 1.0f;
    float ndcY = (pt.y / height) * 2.0f - 1.0f;

    return {ndcX, ndcY};
}

struct FrameInfo {
    float width;
    float height;
};

struct GlyphMeta {
    simd_float2 offset;
};

struct TextUniforms {
    float3 color;
};

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 offset [[attribute(1)]];
    int metadataIndex [[attribute(2)]];
};


struct VertexOut {
    float4 position [[position]];
    float4 worldPosition;
    int metadataIndex [[flat]];
};

vertex VertexOut vertex_text(
    VertexIn in [[stage_in]],
    constant FrameInfo* frameInfo [[buffer(1)]]
)
{
    VertexOut out;
    
    float2 adjustedPos = (in.position+in.offset)/64.0f;
    float2 ndcPos = toNDC(adjustedPos, frameInfo->width, frameInfo->height);
    out.position = float4(ndcPos, 0.0, 1.0);
    out.worldPosition = float4(in.position, 0.0, 1.0);
    out.metadataIndex = in.metadataIndex;
    
    return out;
}

int countIntersections(float2 p0, float2 p1, float2 p2, float fragX, float fragY) {
    float eps = 1e-6;
    // bezier quadratic: (P_0-2P_1+P_2)t^2+(P_1-P_2)t+P_0
    
    float minY = min(min(p0.y, p1.y), p2.y);
    float maxY = max(max(p0.y, p1.y), p2.y);
    if (fragY < minY || fragY >= maxY) return 0;
    
    // (-b +- sqrt(b^2-4ac))/2a
    
    float a = p0.y-2.0f*p1.y+p2.y;
    float b = 2.0f*(p1.y-p0.y);
    float c = p0.y - fragY; // solving for f(x) = fragY

    int intersections = 0;
    
    if (fabs(a) < eps) { // degens to line
        if (fabs(b) < eps) return 0;
        
        float t = -c / b;
        
        if (fabs(t) < eps) t = 0.0f;
        if (fabs(1.0-t) < eps) t = 1.0f;
        
        if (t >= 0.0 && t <= 1.0) {
            float x = mix(mix(p0.x, p1.x, t), mix(p1.x, p2.x, t), t);
            
            if (x > fragX) ++intersections;
        }
        
    }else {
        float discrim = b*b-4.0f*a*c;
        if (discrim >= 0.0) {
            float sqDiscrim = sqrt(discrim);
            float inv = 1.0f/(2.0f*a);
            float t1 = (-b + sqDiscrim) * inv;
            float t2 = (-b - sqDiscrim) * inv;
            
            if (fabs(t1) < eps) t1 = 0.0f;
            if (fabs(1.0-t1) < eps) t1 = 1.0f;
            
            if (fabs(t2) < eps) t2 = 0.0f;
            if (fabs(1.0-t2) < eps) t2 = 1.0f;
            
            if (t1 >= 0.0 && t1 <= 1.0) {
                float x = (p0.x-2.0f*p1.x+p2.x)*t1*t1+2.0f*(p1.x-p0.x)*t1+p0.x;
                
                if (x > fragX) ++intersections;
            }
            
            if (t2 >= 0.0 && t2 <= 1.0) {
                float x = (p0.x-2.0f*p1.x+p2.x)*t2*t2+2.0f*(p1.x-p0.x)*t2+p0.x;
                
                if (x > fragX) ++intersections;
            }
        }
    }
    
    return intersections;
}

// newton approximation distance
float approximateDistance(float2 p0, float2 p1, float2 p2, float2 q) {
    float2 a = p0 - 2.0*p1 + p2;
    float2 b = 2.0*(p1 - p0);
    float2 c = p0;

    float2 chord = p2 - p0;
    float t = clamp(dot(q - p0, chord) / dot(chord,chord), 0.0, 1.0);

    for (int i=0; i<2; ++i) {
        float2 B  = (a*t + b)*t + c;
        float2 dB = 2.0*a*t + b;
        float2 r  = B - q;
        float f   = dot(r, dB);
        float df  = dot(dB, dB) + dot(r, 2.0*a);
        t = clamp(t - f/df, 0.0, 1.0);
    }

    float2 B = (a*t + b)*t + c;
    return distance(q, B);
}

fragment float4 fragment_text(
    VertexOut in [[stage_in]],
    constant float2* bezierPoints [[buffer(0)]],
    constant int* glyphMeta [[buffer(1)]],
    constant TextUniforms* uniforms [[buffer(2)]]
)
{
    float4 fragPt = in.worldPosition;
    int metadataIndex = in.metadataIndex;
    int bezierIndex = glyphMeta[metadataIndex];
    int numContours = glyphMeta[metadataIndex+1];
    float minDist = 1e20;
    
    int intersections = 0;
    int coff = 0;
    for (int ci = 0; ci < numContours; ++ci) {
        int contourSize = glyphMeta[metadataIndex+2+ci];
        
        for (int cpi = 0; cpi < contourSize; cpi += 3, coff += 3) {
            auto p0 = bezierPoints[bezierIndex+coff];
            auto p1 = bezierPoints[bezierIndex+coff+1];
            auto p2 = bezierPoints[bezierIndex+coff+2];
            
            minDist = min(minDist, approximateDistance(p0, p1, p2, fragPt.xy));
            
            intersections += countIntersections(p0, p1, p2, fragPt.x, fragPt.y);
        }
    }
    
    bool inside = intersections & 1;

    
    float sd = inside ? -minDist : minDist;
    
    float px = fwidth(sd);

    float alpha = clamp(0.5 - sd/px, 0.0, 1.0);

    float3 rgb = mix(float3{uniforms->color}, float3{uniforms->color}, alpha);

    return float4(rgb*alpha,alpha);
}
