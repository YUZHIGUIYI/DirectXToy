#include "common.hlsli"

// Vertex shader
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);
    float3 normalW = mul(vIn.normalL, (float3x3)g_WorldInvTranspose);

    // If draw a reflective obeject, perform the reflection operation first
    [flatten]
    if (g_IsReflection)
    {
        posW = mul(posW, g_Reflection);
        normalW = mul(normalW, (float3x3)g_Reflection);
    }

    vOut.posH = mul(posW, viewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = normalW;
    vOut.tex = vIn.tex;
    return vOut;
}
