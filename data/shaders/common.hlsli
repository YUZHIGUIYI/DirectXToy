cbuffer ConstantBuffer : register(b0)
{
    matrix g_World;     // column-matrix by default, unless it has specificed by "row_major"
    matrix g_View;      // matrix == float4x4
    matrix g_Proj;      
}

struct VertexIn
{
    float3 posL : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};