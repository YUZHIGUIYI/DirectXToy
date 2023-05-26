cbuffer CBChangeEveryDraw : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;    
}

cbuffer CBDrawStates : register(b1)
{
    int g_IsReflection;
    float3 g_Pad1;
}

cbuffer CBChangeEveryFrame : register(b2)
{
    matrix g_View;
    float3 g_EyePosW;
}

cbuffer CBChangeOnResize : register(b3)
{
    matrix g_Proj;
}

cbuffer CBChangeRarely : register(b4)
{
    matrix g_Reflection;
}

Texture2D g_Texture : register(t0);
SamplerState g_SamLinear : register(s0);

struct VertexPosNormalTex
{
    float3 posL : POSITION;
    float3 normalL : NORMAL;
    float2 tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 tex : TEXCOORD;
};