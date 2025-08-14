//
//  shaders.metal
//  font-rendeirng
//
//  Created by Taanish Reja on 7/23/25.
//

#include <metal_stdlib>
using namespace metal;

struct FragConstants {
    unsigned long nPoints;
    unsigned long numContours;
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
    VertexIn in [[stage_in]]
)
{
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.worldPos = in.position;
    return out;
}

//fragment float4 fragment_main(
//    VertexOut in [[stage_in]],
//    constant float2* vertices [[buffer(1)]],
//    constant FragConstants& constants [[buffer(2)]],
//    constant ContourBounds* contourBounds [[buffer(3)]]
//)
//{
//    float x = in.worldPos.x;
//    float y = in.worldPos.y;
//    
//    
//    int intersections = 0;
//    
//    
//    for (unsigned long c = 0; c < constants.numContours; ++c) {
//        ContourBounds cb = contourBounds[c];
//
//        unsigned long offset = cb.start;
//        unsigned long contourSize = cb.end-cb.start;
//        
//        for (unsigned long p = 0; p < contourSize; ++p) {
//            unsigned long edgeStart = offset + p;
//            unsigned long edgeEnd = offset + (p+1)%contourSize;
//            
//            float2 v1 = vertices[edgeStart];
//            float2 v2 = vertices[edgeEnd];
//            
//            if (v1.y > y != v2.y > y) {
//                float intersectX = v1.x + (y - v1.y) * ((v2.x - v1.x) / (v2.y - v1.y));
//                if (intersectX > x) {
//                    ++intersections;
//                }
//            }
//            
//            
//        }
//    }
//   
//    if (intersections % 2 == 0)
//        return float4(1,1,1,1);
//    else
//        return float4(0,0,0,1);
//}
//

fragment float4 fragment_main(
    VertexOut in [[stage_in]],
    constant float2* vertices         [[buffer(1)]],
    constant FragConstants& constants [[buffer(2)]],
    constant ContourBounds* contourBounds [[buffer(3)]]
)
{
    constexpr int GRID = 8;
    constexpr int SAMPLES = GRID * GRID;

    // Size of each subpixel step in your world units
    const float step = 0.0005 / float(GRID); // 0.0005 total span (like your ±0.00025 example)

    const float4 FG = float4(0.0, 0.0, 0.0, 1.0);
    const float4 BG = float4(1.0, 1.0, 1.0, 1.0);

    auto insideAt = [&](float px, float py) -> bool {
        int intersections = 0;

        for (ulong i = 0; i < constants.numContours; ++i) {
            ContourBounds cb = contourBounds[i];
            ulong offset = cb.start;
            ulong contourSize = cb.end - cb.start;

            for (ulong p = 0; p < contourSize; ++p) {
                ulong edgeStart = offset + p;
                ulong edgeEnd   = offset + ((p + 1) % contourSize);

                float2 v1 = vertices[edgeStart];
                float2 v2 = vertices[edgeEnd];

                if ((v1.y > py) != (v2.y > py)) {
                    float intersectX = v1.x + (py - v1.y) * ((v2.x - v1.x) / (v2.y - v1.y));
                    if (intersectX > px) {
                        ++intersections;
                    }
                }
            }
        }

        return (intersections & 1) != 0;
    };

    float coverage = 0.0;
    float2 base = in.worldPos.xy;

    // Build the grid centered around the pixel
    for (int sy = 0; sy < GRID; ++sy) {
        for (int sx = 0; sx < GRID; ++sx) {
            float offsetX = (float(sx) - (GRID - 1) / 2.0) * step;
            float offsetY = (float(sy) - (GRID - 1) / 2.0) * step;

            float2 sp = base + float2(offsetX, offsetY);
            coverage += insideAt(sp.x, sp.y) ? 1.0 : 0.0;
        }
    }

    coverage /= float(SAMPLES);
    return BG * (1.0 - coverage) + FG * coverage;
}
