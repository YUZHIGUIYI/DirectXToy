#include "light_and_material.hlsli"

Texture2D g_Texture : register(t0);
SamplerState g_SamLinear : register(s0);

cbuffer CBChangeEveryDraw : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose; 
    Material g_Material;
}

cbuffer CBDrawStates : register(b1)
{
    int g_IsReflection;
    int g_IsShadow;
    float2 g_Pad1;
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
    matrix g_Shadow;
    matrix g_RefShadow;
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