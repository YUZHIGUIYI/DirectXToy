#ifndef _SHADOW_VS_
#define _SHADOW_VS_

#include "shadow_cb.hlsl"
#include "shadow_vertex_definitions.hlsl"

VertexShaderOutput VS(VertexShaderInput vin)
{
    VertexShaderOutput vout;

    vout.homog_position = mul(float4(vin.position, 1.0f), gWorldViewProj);
    vout.texcoord = vin.texcoord;

    return vout;
}

#endif 