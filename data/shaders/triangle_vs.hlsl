#include "triangle.hlsli"

// Vertex shader
VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.posH = float4(vIn.pos, 1.0f);
    vOut.color = vIn.color; // alpha pass is 1.0 by default
    return vOut;
}
