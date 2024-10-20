#ifndef _SKY_LUT_
#define _SKY_LUT_

#include "intersection.hlsl"
#include "medium.hlsl"

Texture2D<float3> gTransmittanceMap   : register(t0);
Texture2D<float3> gMultiScatteringMap : register(t1);
SamplerState gSamMT                   : register(s0);

cbuffer CBPSParams : register(b1)
{
    float3 gAtomsEyePosition;
    int    gMarchStepCount;

    float3 gSunDirection;
    int    gEnableMultiScattering;

    float3 gSunIntensity;
    int    gUnlessPadding;
}

struct VertexShaderOutput
{
    float4 homog_position : SV_POSITION;
    float2 texcoord       : TEXCOORD;
};

VertexShaderOutput VS(uint vertex_id : SV_VertexID)
{
    VertexShaderOutput vs_output;
    vs_output.texcoord = float2((vertex_id << 1) & 2, vertex_id & 2);
    vs_output.homog_position = float4(vs_output.texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.5f, 1.0f);
    return vs_output;
}


void march_step(float phase_u, float3 ori, float3 dir, float this_t, float next_t,
                inout float3 sum_sigma_t, inout float3 in_scattering)
{
    float mid_t = 0.5f * (this_t + next_t);
    float3 pos_r = float3(0.0f, ori.y + gPlanetRadius, 0.0f) + dir * mid_t;
    float height = length(pos_r) - gPlanetRadius;

    float3 sigma_s;
    float3 sigma_t;
    get_sigma_st(height, sigma_s, sigma_t);

    float3 delta_sum_sigma_t = (next_t - this_t) * sigma_t;
    float3 eye_trans = exp(-sum_sigma_t - 0.5f * delta_sum_sigma_t);

    float sun_theta = PI / 2.0f - acos(dot(-gSunDirection, normalize(pos_r)));

    if (!has_intersection_with_sphere(pos_r, -gSunDirection, gPlanetRadius))
    {
        float3 rho = eval_phase_function(height, phase_u);
        float3 sun_trans = get_transmittance(gTransmittanceMap, gSamMT, height, sun_theta);
        in_scattering += (next_t - this_t) * eye_trans * sigma_s * rho * sun_trans;
    }

    if (gEnableMultiScattering)
    {
        float tx = height / (gAtomsphereRadius - gPlanetRadius);
        float ty = 0.5f + 0.5f * sin(sun_theta);
        float3 ms = gMultiScatteringMap.SampleLevel(gSamMT, float2(tx, ty), 0.0f);
        in_scattering += (next_t - this_t) * eye_trans * sigma_s * ms;
    }

    sum_sigma_t += delta_sum_sigma_t;
}

float4 PS(VertexShaderOutput ps_input) : SV_Target0
{
    float phi = 2.0f * PI * ps_input.texcoord.x;
    float vm = 2.0f * ps_input.texcoord.y - 1.0f;
    float theta = sign(vm) * (PI / 2.0f) * vm * vm;
    float sin_theta = sin(theta);
    float cos_theta = cos(theta);

    float3 ori = gAtomsEyePosition;
    float3 dir = float3(cos(phi) * cos_theta, sin_theta, sin(phi) * cos_theta);

    float2 planet_ori = float2(0.0f, ori.y + gPlanetRadius);
    float2 planet_dir = float2(cos_theta, sin_theta);

    // Find end point
    float end_t = 0.0f;
    if (!find_closest_intersection_with_circle(planet_ori, planet_dir, gPlanetRadius, end_t))
    {
        find_closest_intersection_with_circle(planet_ori, planet_dir, 
                                                gAtomsphereRadius, end_t);
    }

    // Phase function input
    float phase_u = dot(gSunDirection, -dir);

    // Ray march
    float t = 0.0f;
    float3 in_scatter = float3(0.0f, 0.0f, 0.0f);
    float3 sum_sigma_t = float3(0.0f, 0.0f, 0.0f);
    
    float dt = (end_t - t) / gMarchStepCount;
    for (int i = 0; i < gMarchStepCount; ++i)
    {
        float next_t = t + dt;
        march_step(phase_u, ori, dir, t, next_t, sum_sigma_t, in_scatter);
        t = next_t;
    }

    return float4(in_scatter * gSunIntensity, 1.0f);
}

#endif