#include "common.hlsli"

float4 PS(VertexPosHTex pIn) : SV_Target
{
    float4 color = g_Texture.Sample(g_SamLinear, pIn.tex);
    //clip(color.a - 0.1f);
    return color;
}