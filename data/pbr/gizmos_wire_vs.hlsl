#ifndef _GIZMOS_WIRE_VS_
#define _GIZMOS_WIRE_VS_

struct VertexShaderInput
{
    float3 position       : POSITION;
};

struct VertexShaderOutput
{
    float4 homog_position : SV_POSITION;
};

cbuffer CBTramsform       : register(b0)
{
    matrix gWorldViewProj;
}

VertexShaderOutput VS(VertexShaderInput vin)
{
    VertexShaderOutput vout;

    vout.homog_position = mul(float4(vin.position, 1.0f), gWorldViewProj);

    return vout;
}

#endif