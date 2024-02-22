#ifndef _GIZMOS_WIRE_PS_
#define _GIZMOS_WIRE_PS_

struct VertexShaderOutput
{
    float4 homog_position  : SV_POSITION;
};

cbuffer CBWireColorControl : register(b1)
{
    float4 gWireColor;
}

float4 PS(VertexShaderOutput pin) : SV_Target
{
    // Currently do not consider alpha blend
    return float4(gWireColor.rgb, 1.0f);
}

#endif