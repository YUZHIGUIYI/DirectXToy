#include "common.hlsli"

// Vertex shader
VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.posH = mul(float4(vIn.posL, 1.0f), g_World);   // Cij = Aij * Bij
    vOut.posH = mul(vOut.posH, g_View);
    vOut.posH = mul(vOut.posH, g_Proj);
    vOut.color = vIn.color; // alpha pass is 1.0 by default
    return vOut;
}
