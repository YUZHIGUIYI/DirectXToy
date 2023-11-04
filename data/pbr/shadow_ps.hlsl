#ifndef _SHADOW_PS_
#define _SHADOW_PS_

#include "shadow_vertex_definitions.hlsl"
#include "samplers.hlsl"

Texture2D gAlbedoMap : register(t0);

// This is only used for alpha geometric clipping to ensure the correct display of shadow
// For geometry that does not require texture sampling operation, can be directly set to nullptr in pixel shader

void PS(VertexShaderOutput pin, uniform float clip_value)
{
    float4 diffuse = gAlbedoMap.Sample(gSamLinearWrap, pin.texcoord);

    // Do not write transparent pixels to depth map
    clip(diffuse.a - clip_value);
}

float4 DebugPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float depth = gAlbedoMap.Sample(gSamLinearWrap, texcoord).r;

    depth = 1.0f - depth;

    return float4(depth.rrr, 1.0f);
}


#endif