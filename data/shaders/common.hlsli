#include "light_and_material.hlsli"

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sam : register(s0);

cbuffer CBChangeEveryDraw : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose; 
    Material g_Material;
}

cbuffer CBChangeEveryFrame : register(b1)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
}

cbuffer CBChangeRarely : register(b2)
{
    DirectionalLight g_DirLight[2];
    PointLight g_PointLight[2];
}

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

struct VertexPosTex
{
    float3 posL : POSITION;
    float2 tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 posH : SV_POSITION;
    float2 tex : TEXCOORD;
};