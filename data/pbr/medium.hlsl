#ifndef _MEDIUM_
#define _MEDIUM_

#include "intersection.hlsl"

cbuffer CBAtomsphereParams : register(b0)
{
    float3 gScatterRayleigh;
    float  gHDensityRayleigh;

    float  gScatterMie;
    float  gAsymmetryMie;
    float  gAbsorbMie;
    float  gHDensityMie;

    float3 gAbsorbOzone;
    float  gOzoneCenterHeight;

    float  gOzoneThickness;
    float  gPlanetRadius;
    float  gAtomsphereRadius;
    float  gPadding;
}


float3 get_sigma_s(float height)
{
    float3 rayleigh = gScatterRayleigh * exp(-height / gHDensityRayleigh);
    float mie = gScatterMie * exp(-height / gHDensityMie);
    return rayleigh + mie;
}

float3 get_sigma_t(float height)
{
    float3 rayleigh = gScatterRayleigh * exp(-height / gHDensityRayleigh);
    float mie = (gScatterMie + gAbsorbMie) * exp(-height / gHDensityMie);
    float3 ozone = gAbsorbOzone * max(0.0f, 
                    1.0f - 0.5f * abs(height - gOzoneCenterHeight) / gOzoneThickness);
    return rayleigh + mie + ozone;
}

void get_sigma_st(float height, out float3 sigma_s, out float3 sigma_t)
{
    float3 rayleigh = gScatterRayleigh * exp(-height / gHDensityRayleigh);

    float mie_density = exp(-height / gHDensityMie);
    float mie_s = gScatterMie * mie_density;
    float mie_t = (gScatterMie + gAbsorbMie) * mie_density;

    float3 ozone = gAbsorbOzone * max(0.0f,
                    1.0f - 0.5f * abs(height - gOzoneCenterHeight) / gOzoneThickness);
    
    sigma_s = rayleigh + mie_s;
    sigma_t = rayleigh + mie_t + ozone;
}

float3 eval_phase_function(float height, float u)
{
    float3 s_rayleigh = gScatterRayleigh * exp(-height / gHDensityRayleigh);
    float s_mie = gScatterMie * exp(-height / gHDensityMie);
    float3 s = s_rayleigh + s_mie;

    float g = gAsymmetryMie;
    float g2 = g * g;
    float u2 = u * u;
    float p_rayleigh = 3.0f / (16.0f * PI) * (1.0f + u2);

    float m = 1.0f + g2 - 2.0f * g * u;
    float p_mie = 3.0f / (8.0f * PI) * (1.0f - g2) * (1.0f + u2) / ((2.0f + g2) * m * sqrt(m));

    float3 result;
    result.x = s.y > 0.0f ? (p_rayleigh * s_rayleigh.x + p_mie * s_mie) / s.x : 0.0f;
    result.y = s.y > 0.0f ? (p_rayleigh * s_rayleigh.y + p_mie * s_mie) / s.y : 0.0f;
    result.z = s.z > 0.0f ? (p_rayleigh * s_rayleigh.z + p_mie * s_mie) / s.z : 0.0f;
    return result;
}

float3 get_transmittance(Texture2D<float3> tex, SamplerState samp, float height, float theta)
{
    float u = height / (gAtomsphereRadius - gPlanetRadius);
    float v = 0.5f + 0.5f * sin(theta);
    return tex.SampleLevel(samp, float2(u, v), 0.0f);
}

#endif