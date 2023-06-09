#include "common.hlsli"

// Vertex shader
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;

    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);

    vOut.posW = posW.xyz;
    vOut.posH = mul(posW, g_ViewProj);
    vOut.normalW = mul(vIn.normalL, (float3x3)g_WorldInvTranspose);
    vOut.tex = vIn.tex;
    return vOut;
}
