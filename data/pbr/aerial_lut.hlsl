#ifndef _AERIAL_LUT_
#define _AERIAL_LUT_

#include "intersection.hlsl"
#include "medium.hlsl"

#define THREAD_GROUP_SIZE_X 16
#define THREAD_GROUP_SIZE_Y 16

cbuffer CBCSParams : register(b1)
{
    float3 gSunDirection;
    float  gSunTheta;

    float3 gFrustumA;
    float  gMaxDistance;

    float3 gFrustumB;
    int    gPerSliceMarchStepCount;

    float3 gFrustumC;
    int    gEnableMultiScattering;

    float3 gFrustumD;
    float  gAtomsEyeHeight;

    float3 gEyePosition;
    int    gEnableShadow;

    float4x4 gShadowViewProj;
    float    gWorldScale;
}

Texture2D<float3> gMultiScatteringMap : register(t0);
Texture2D<float3> gTransmittanceMap   : register(t1);
Texture2D<float>  gShadowMap          : register(t2);

SamplerState      gSamMT              : register(s0);
SamplerState      gSamShadow          : register(s1);

RWTexture3D<float4> gAerialPerspectiveLUT : register(u0);

float relative_luminance(float3 c)
{
    return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
}

[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, 1)]
void CS(uint3 thread_idx : SV_DispatchThreadID)
{
    uint width, height, depth;
    gAerialPerspectiveLUT.GetDimensions(width, height, depth);
    if (thread_idx.x >= width || thread_idx.y >= height)
    {
        return;
    }

    float xf = (thread_idx.x + 0.5f) / width;
    float yf = (thread_idx.y + 0.5f) / height;

    float3 ori = float3(0.0f, gAtomsEyeHeight, 0.0f);
    float3 dir = normalize(lerp(
        lerp(gFrustumA, gFrustumB, xf),
        lerp(gFrustumC, gFrustumD, xf),
        yf
    ));

    float u = dot(gSunDirection, -dir);

    float max_t = 0.0f;
    if (!find_closest_intersection_with_sphere(ori + float3(0.0f, gPlanetRadius, 0.0f), 
        dir, gPlanetRadius, max_t))
    {
        find_closest_intersection_with_sphere(ori + float3(0.0f, gPlanetRadius, 0.0f), 
            dir, gAtomsphereRadius, max_t);
    }

    float slice_depth = gMaxDistance / depth;
    float half_slice_depth = 0.5f * slice_depth;
    float t_beg = 0.0f;
    float t_end = min(half_slice_depth, max_t);

    float3 sum_sigma_t = float3(0.0f, 0.0f, 0.0f);
    float3 in_scatter = float3(0.0f, 0.0f, 0.0f);
    
    float rand = frac(sin(dot(
        float2(xf, yf), float2(12.9898f, 78.233f) * 2.0f)) * 43758.5453f);
    
    for (int z = 0; z < depth; ++z)
    {
        float dt = (t_end - t_beg) / gPerSliceMarchStepCount;
        float t = t_beg;

        for (int i = 0; i < gPerSliceMarchStepCount; ++i)
        {
            float next_t = t + dt;

            float mid_t = lerp(t, next_t, rand);
            float3 pos_r = float3(0.0f, ori.y + gPlanetRadius, 0.0f) + dir * mid_t;
            float h = length(pos_r) - gPlanetRadius;

            float3 sigma_s, sigma_t;
            get_sigma_st(h, sigma_s, sigma_t);

            float3 delta_sum_sigma_t = dt * sigma_t;
            float3 eye_trans = exp(-sum_sigma_t - 0.5f * delta_sum_sigma_t);

            if (!has_intersection_with_sphere(pos_r, -gSunDirection, gPlanetRadius))
            {
                float3 shadow_pos = gEyePosition + dir * mid_t / gWorldScale;
                float4 shadow_clip = mul(float4(shadow_pos, 1.0f), gShadowViewProj);
                float2 shadow_ndc = shadow_clip.xy / shadow_clip.w;
                float2 shadow_uv = 0.5f + float2(0.5f, -0.5f) * shadow_ndc;

                bool in_shadow = gEnableShadow;
                if (gEnableShadow && all(saturate(shadow_uv) == shadow_uv))
                {
                    float ray_z = shadow_clip.z;
                    float sm_z = gShadowMap.SampleLevel(gSamShadow, shadow_uv, 0.0f);
                    in_shadow = ray_z >= sm_z;
                }

                if (!in_shadow)
                {
                    float3 rho = eval_phase_function(h, u);
                    float3 sun_trans = get_transmittance(gTransmittanceMap, gSamMT, h, gSunTheta);
                    in_scatter += dt * eye_trans * sigma_s * rho * sun_trans;
                }
            }

            if (gEnableMultiScattering)
            {
                float tx = h / (gAtomsphereRadius - gPlanetRadius);
                float ty = 0.5f + 0.5f * sin(gSunTheta);
                float3 ms = gMultiScatteringMap.SampleLevel(gSamMT, float2(tx, ty), 0.0f);
                in_scatter += dt * eye_trans * sigma_s * ms;
            }

            sum_sigma_t += delta_sum_sigma_t;
            t = next_t;
        }

        float transmittance = relative_luminance(exp(-sum_sigma_t));
        gAerialPerspectiveLUT[uint3(thread_idx.xy, z)] = 
            float4(in_scatter, transmittance);

        t_beg = t_end;
        t_end = min(t_end + slice_depth, max_t);
    }
}

#endif