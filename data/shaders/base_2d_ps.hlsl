#include "common.hlsli"

float4 PS(VertexPosHTex pIn) : SV_Target
{
    float4 color = g_DiffuseMap.Sample(g_Sam, pIn.tex);
    //clip(color.a - 0.1f);
    return color;
}