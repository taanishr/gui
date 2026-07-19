//
//  shaders.metal
//  font-rendeirng
//
//  Created by Taanish Reja on 7/23/25.
//

#include <metal_stdlib>
#include "common.metal"

using namespace metal;
struct TextUniforms {
    float4 color;
    float fontSize;
    uint numClips;
};

struct TextVertexIn {
    float2 position [[attribute(0)]];
    int metadataIndex [[attribute(1)]];
    int atom_id [[attribute(2)]];
    float2 shapingOffset [[attribute(3)]];
};


struct TextVertexOut {
    float4 position [[position]];
    float4 worldPosition;
    float4 clipPosition;
    int metadataIndex [[flat]];
};

vertex TextVertexOut vertex_text(
    TextVertexIn in [[stage_in]],
    constant float2* offsets [[buffer(1)]],
    constant FrameInfo* frameInfo [[buffer(2)]],
    constant TextUniforms* uniforms [[buffer(3)]]
)
{
    TextVertexOut out;

    float scale = uniforms->fontSize/BASE_PIXEL_HEIGHT;

    float2 adjustedPos = ((in.position + in.shapingOffset) * scale)/64.0f + offsets[in.atom_id];
    float2 ndcPos = toNDC(adjustedPos, frameInfo->width, frameInfo->height);
    out.position = float4(ndcPos, 0.0, 1.0);
    out.worldPosition = float4(in.position, 0.0, 1.0);
    out.clipPosition = float4(adjustedPos, 0.0, 1.0);
    out.metadataIndex = in.metadataIndex;
    
    return out;
}

float2 evaluateQuadratic(float2 p0, float2 p1, float2 p2, float t) {
    float u = 1.0 - t;
    return u * u * p0 + 2.0 * u * t * p1 + t * t * p2;
}

float2 quadraticDerivative(float2 p0, float2 p1, float2 p2, float t) {
    return 2.0 * ((1.0 - t) * (p1 - p0) + t * (p2 - p1));
}

float2 quadraticSecondDerivative(float2 p0, float2 p1, float2 p2) {
    return 2.0 * (p2 - 2.0 * p1 + p0);
}

float2 evaluateCubic(float2 p0, float2 p1, float2 p2, float2 p3, float t) {
    float u = 1.0 - t;
    return u * u * u * p0
        + 3.0 * u * u * t * p1
        + 3.0 * u * t * t * p2
        + t * t * t * p3;
}

float2 cubicDerivative(float2 p0, float2 p1, float2 p2, float2 p3, float t) {
    float u = 1.0 - t;
    return 3.0 * u * u * (p1 - p0)
        + 6.0 * u * t * (p2 - p1)
        + 3.0 * t * t * (p3 - p2);
}

float2 cubicSecondDerivative(float2 p0, float2 p1, float2 p2, float2 p3, float t) {
    return 6.0 * (1.0 - t) * (p2 - 2.0 * p1 + p0)
        + 6.0 * t * (p3 - 2.0 * p2 + p1);
}

int countQuadraticIntersections(float2 p0, float2 p1, float2 p2, float fragX, float fragY) {
    float eps = 1e-5;
    
    float minY = min(min(p0.y, p1.y), p2.y);
    float maxY = max(max(p0.y, p1.y), p2.y);
    if (fragY < minY || fragY >= maxY) return 0;
    
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
            float x = evaluateQuadratic(p0, p1, p2, t).x;
            
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
                float x = evaluateQuadratic(p0, p1, p2, t1).x;
                
                if (x > fragX) ++intersections;
            }
            
            if (t2 >= 0.0 && t2 <= 1.0) {
                float x = evaluateQuadratic(p0, p1, p2, t2).x;
                
                if (x > fragX) ++intersections;
            }
        }
    }
    
    return intersections;
}

float approximateQuadraticDistance(float2 p0, float2 p1, float2 p2, float2 q) {
    float2 chord = p2 - p0;
    float chordLengthSquared = dot(chord, chord);
    float t = chordLengthSquared > 1e-6
        ? clamp(dot(q - p0, chord) / chordLengthSquared, 0.0, 1.0)
        : 0.5;

    for (int i=0; i<2; ++i) {
        float2 point = evaluateQuadratic(p0, p1, p2, t);
        float2 first = quadraticDerivative(p0, p1, p2, t);
        float2 second = quadraticSecondDerivative(p0, p1, p2);
        float2 residual = point - q;
        float denominator = dot(first, first) + dot(residual, second);
        if (abs(denominator) < 1e-6) break;
        t = clamp(t - dot(residual, first) / denominator, 0.0, 1.0);
    }

    return min(
        distance(q, evaluateQuadratic(p0, p1, p2, t)),
        min(distance(q, p0), distance(q, p2))
    );
}

float approximateCubicDistance(float2 p0, float2 p1, float2 p2, float2 p3, float2 q) {
    float2 chord = p3 - p0;
    float chordLengthSquared = dot(chord, chord);
    float t = chordLengthSquared > 1e-6
        ? clamp(dot(q - p0, chord) / chordLengthSquared, 0.0, 1.0)
        : 0.5;

    for (int i = 0; i < 2; ++i) {
        float2 point = evaluateCubic(p0, p1, p2, p3, t);
        float2 first = cubicDerivative(p0, p1, p2, p3, t);
        float2 second = cubicSecondDerivative(p0, p1, p2, p3, t);
        float2 residual = point - q;
        float denominator = dot(first, first) + dot(residual, second);
        if (abs(denominator) < 1e-6) break;
        t = clamp(t - dot(residual, first) / denominator, 0.0, 1.0);
    }

    return min(
        distance(q, evaluateCubic(p0, p1, p2, p3, t)),
        min(distance(q, p0), distance(q, p3))
    );
}

int countCubicIntersections(
    float2 p0,
    float2 p1,
    float2 p2,
    float2 p3,
    float fragX,
    float fragY
) {
    float bounds[4] = {0.0, 1.0, 1.0, 1.0};
    int boundCount = 1;
    float a = -p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y;
    float b = 2.0 * (p0.y - 2.0 * p1.y + p2.y);
    float c = p1.y - p0.y;
    constexpr float epsilon = 1e-6;

    if (abs(a) < epsilon) {
        if (abs(b) >= epsilon) {
            float root = -c / b;
            if (root > 0.0 && root < 1.0) bounds[boundCount++] = root;
        }
    } else {
        float discriminant = b * b - 4.0 * a * c;
        if (discriminant >= 0.0) {
            float rootDiscriminant = sqrt(discriminant);
            float root1 = (-b - rootDiscriminant) / (2.0 * a);
            float root2 = (-b + rootDiscriminant) / (2.0 * a);
            if (root1 > 0.0 && root1 < 1.0) bounds[boundCount++] = root1;
            if (root2 > 0.0 && root2 < 1.0 && abs(root2 - root1) >= epsilon) {
                bounds[boundCount++] = root2;
            }
        }
    }

    bounds[boundCount++] = 1.0;
    for (int i = 1; i < boundCount; ++i) {
        float value = bounds[i];
        int j = i;
        while (j > 0 && bounds[j - 1] > value) {
            bounds[j] = bounds[j - 1];
            --j;
        }
        bounds[j] = value;
    }

    int intersections = 0;
    for (int interval = 0; interval < boundCount - 1; ++interval) {
        float lower = bounds[interval];
        float upper = bounds[interval + 1];
        float lowerY = evaluateCubic(p0, p1, p2, p3, lower).y;
        float upperY = evaluateCubic(p0, p1, p2, p3, upper).y;
        if (fragY < min(lowerY, upperY) || fragY >= max(lowerY, upperY)) continue;

        bool increasing = upperY > lowerY;
        for (int iteration = 0; iteration < 8; ++iteration) {
            float midpoint = (lower + upper) * 0.5;
            float midpointY = evaluateCubic(p0, p1, p2, p3, midpoint).y;
            if ((midpointY < fragY) == increasing) lower = midpoint;
            else upper = midpoint;
        }

        float root = (lower + upper) * 0.5;
        if (evaluateCubic(p0, p1, p2, p3, root).x > fragX) ++intersections;
    }

    return intersections;
}


fragment float4 fragment_text(
    TextVertexOut in [[stage_in]],
    constant float2* bezierPoints [[buffer(0)]],
    constant int* glyphMeta [[buffer(1)]],
    constant TextUniforms* uniforms [[buffer(2)]],
    constant ClipUniform* clips [[buffer(3)]]
)
{
    if (outside_clips(in.clipPosition.xy, clips, uniforms->numClips)) {
        discard_fragment();
    }

    float4 fragPt = in.worldPosition;

    int metadataIndex = in.metadataIndex;
    int bezierIndex = glyphMeta[metadataIndex];
    CurveType curveType = CurveType(uint(glyphMeta[metadataIndex + 1]));
    int numContours = glyphMeta[metadataIndex + 2];
    int pointStride = curveType == CurveType::Cubic ? 4 : 3;
    float minDist = 1e20;
    
    int intersections = 0;
    int coff = 0;

    for (int ci = 0; ci < numContours; ++ci) {
        int contourSize = glyphMeta[metadataIndex + 3 + ci];
        
        for (int cpi = 0; cpi < contourSize; cpi += pointStride, coff += pointStride) {
            auto p0 = bezierPoints[bezierIndex+coff];
            auto p1 = bezierPoints[bezierIndex+coff+1];
            auto p2 = bezierPoints[bezierIndex+coff+2];

            if (curveType == CurveType::Cubic) {
                auto p3 = bezierPoints[bezierIndex+coff+3];
                minDist = min(
                    minDist,
                    approximateCubicDistance(p0, p1, p2, p3, fragPt.xy)
                );
                intersections += countCubicIntersections(
                    p0, p1, p2, p3, fragPt.x, fragPt.y
                );
            } else {
                minDist = min(
                    minDist,
                    approximateQuadraticDistance(p0, p1, p2, fragPt.xy)
                );
                intersections += countQuadraticIntersections(
                    p0, p1, p2, fragPt.x, fragPt.y
                );
            }
        }
    }

    bool inside = intersections & 1;

    float sd = inside ? -minDist : minDist;

    float px = fwidth(fragPt.x);

    float coverage = clamp(0.5 - sd/px, 0.0, 1.0);

    float alpha = coverage * uniforms->color.w;

    float3 rgb = uniforms->color.rgb;
    return float4(rgb * alpha, alpha);
}
