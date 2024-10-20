#ifndef _MULTISCATTER_
#define _MULTISCATTER_

#include "intersection.hlsl"
#include "medium.hlsl"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

cbuffer CBCSParams : register(b1)
{
    float3 gTerrainAlbedo;
    int    gDirSampleCount;

    float3 gSunIntensity;
    int    gRayMarchStepCount;
}

StructuredBuffer<float2> gRawDirSamples      : register(t0);
Texture2D<float3>        gTransmittanceMap   : register(t1);

RWTexture2D<float4>      gMultiScatteringMap : register(u0);
SamplerState             gSamTransmittance   : register(s0);

float3 uniform_on_unit_sphere(float u1, float u2)
{
    float z = 1.0f - 2.0f * u1;
    float r = sqrt(max(0.0f, 1 - z * z));
    float phi = 2.0f * PI * u2;
    return float3(r * cos(phi), r * sin(phi), z);
}

void integrate(float3 world_ori, float3 world_dir, float sun_theta, 
                float3 to_sun_dir, out float3 inner_l2, out float3 inner_f)
{
    float u = dot(world_dir, to_sun_dir);
    float end_t = 0.0f;
    bool ground_inct = find_closest_intersection_with_sphere(world_ori, world_dir,
                        gPlanetRadius, end_t);
    if (!ground_inct)
    {
        find_closest_intersection_with_sphere(world_ori, world_dir, gAtomsphereRadius, end_t);
    }

    float dt = end_t / gRayMarchStepCount;
    float half_dt = 0.5f * dt;
    float t = 0.0f;

    float3 sum_sigma_t = float3(0.0f, 0.0f, 0.0f);
    float3 sum_l2 = float3(0.0f, 0.0f, 0.0f);
    float3 sum_f = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < gRayMarchStepCount; ++i)
    {
        float mid_t = t + half_dt;
        t += dt;

        float3 world_pos = world_ori + mid_t * world_dir;
        float height = length(world_pos) - gPlanetRadius;

        float3 sigma_s, sigma_t;
        get_sigma_st(height, sigma_s, sigma_t);

        float3 delta_sum_sigma_t = dt * sigma_t;
        float3 transmittance = exp(-sum_sigma_t - 0.5f * delta_sum_sigma_t);

        if (!has_intersection_with_sphere(world_pos, to_sun_dir, gPlanetRadius))
        {
            float3 rho = eval_phase_function(height, u);
            float3 sum_transmittance = get_transmittance(gTransmittanceMap, gSamTransmittance, 
                                        height, sun_theta);
            sum_l2 += dt * transmittance * sum_transmittance * sigma_s * 
                        rho * gSunIntensity;
        }

        sum_f += dt * transmittance * sigma_s;
        sum_sigma_t += delta_sum_sigma_t;
    }

    if (ground_inct)
    {
        float3 transmittance = exp(-sum_sigma_t);
        float3 sun_transmittance = get_transmittance(gTransmittanceMap, gSamTransmittance, 
                                    0.0f, sun_theta);
        sum_l2 += transmittance * sun_transmittance * max(0.0f, to_sun_dir.y) * 
                    gSunIntensity * (gTerrainAlbedo / PI);
    }

    inner_l2 = sum_l2;
    inner_f = sum_f;
}

float3 compute_m(float height, float sun_theta)
{
    float3 world_ori = float3(0.0f, gPlanetRadius + height, 0.0f);
    float3 to_sun_dir = float3(cos(sun_theta), sin(sun_theta), 0.0f);

    float3 sum_l2 = float3(0.0f, 0.0f, 0.0f);
    float3 sum_f = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < gDirSampleCount; ++i)
    {
        float2 raw_sample = gRawDirSamples[i];
        float3 world_dir = uniform_on_unit_sphere(raw_sample.x, raw_sample.y);

        float3 inner_l2 = float3(0.0f, 0.0f, 0.0f);
        float3 inner_f = float3(0.0f, 0.0f, 0.0f);
        integrate(world_ori, world_dir, sun_theta, to_sun_dir, inner_l2, inner_f);

        // Phase function is canceled bu pdf
        sum_l2 += inner_l2;
        sum_f += inner_f;
    }

    float3 l2 = sum_l2 / gDirSampleCount;
    float3 f = sum_f / gDirSampleCount;
    return l2 / (1.0f - f);
}

[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CS(uint3 thread_idx : SV_DispatchThreadID)
{
    uint width, height;
    gMultiScatteringMap.GetDimensions(width, height);
    if (thread_idx.x >= width || thread_idx.y >= height)
    {
        return;
    }

    float sin_sun_theta = lerp(-1.0f, 1.0f, (thread_idx.y + 0.5f) / height);
    float sun_theta = asin(sin_sun_theta);

    float h = lerp(0.0f, gAtomsphereRadius - gPlanetRadius, 
                (thread_idx.x + 0.5f) / width);
    gMultiScatteringMap[thread_idx.xy] = float4(compute_m(h, sun_theta), 1.0f);
}

#endif