#ifndef _TRANSMITTANCE_
#define _TRANSMITTANCE_

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

#define STEP_COUNT 1000

#include "intersection.hlsl"
#include "medium.hlsl"

RWTexture2D<float4> gTransmittanceMap : register(u0);

[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CS(uint3 thread_idx : SV_DispatchThreadID)
{
    uint width, height;
    gTransmittanceMap.GetDimensions(width, height);
    if (thread_idx.x >= width || thread_idx >= height)
    {
        return;
    }
    
    float theta = asin(lerp(-1.0f, 1.0f, (thread_idx.y + 0.5f) / height));
    float h = lerp(0.0f, gAtomsphereRadius - gPlanetRadius, (thread_idx.x + 0.5f) / width);
    float o = float2(0.0f, gPlanetRadius + h);
    float d = float2(cos(theta), sin(theta));
    float t = 0.0f;
    if (!find_closest_intersection_with_circle(o, d, gPlanetRadius, t))
    {
        find_closest_intersection_with_circle(o, d, gAtomsphereRadius, t);
    }
    float2 end = o + t * d;

    float3 sum = 0.0f;
    for (int i = 0; i < STEP_COUNT; ++i)
    {
        float2 pi = lerp(o, end, (i + 0.5f) / STEP_COUNT);
        float height = length(pi) - gPlanetRadius;
        float3 sigma = get_sigma_t(height);
        sum += sigma;
    }

    float3 result = exp(-sum * (t / STEP_COUNT));
    gTransmittanceMap[thread_idx.xy] = float4(result, 1.0f);
}

#endif