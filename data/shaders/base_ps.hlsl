#include "common.hlsli"

// Pixel shader
float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{
    pIn.normalW = normalize(pIn.normalW);

    float3 toEyeW = normalize(g_EyePosW - pIn.posW);

    float4 texColor = g_Texture.Sample(g_SamLinear, pIn.tex);

    return texColor;
}

