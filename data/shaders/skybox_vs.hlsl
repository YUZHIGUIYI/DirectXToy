#include "skybox.hlsli"

VertexPosHL VS(VertexPos vIn)
{
    VertexPosHL vOut;

    float4 posH = mul(float4(vIn.posL, 1.0f), g_WorldViewProj);
    vOut.posH = posH.xyww;
    vOut.posL = vIn.posL;
    return vOut;
}