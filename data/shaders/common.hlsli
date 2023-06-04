#include "light_and_material.hlsli"

Texture2D g_DiffuseMap : register(t0);
TextureCube g_TexCube : register(t1);
SamplerState g_Sam : register(s0);

cbuffer CBChangeEveryInstanceDraw : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose; 
}

cbuffer CBChangeEveryObjectDraw : register(b1)
{
    Material g_Material;
}

cbuffer CBDrawStates : register(b2)
{
    int g_ReflectionEnabled;
    int g_RefractionEnabled;
    float g_Eta;
    float g_Pad1;
}

cbuffer CBChangeEveryFrame : register(b3)
{
    matrix g_ViewProj;
    float3 g_EyePosW;
    float g_Pad2;
}

cbuffer CBChangeRarely : register(b4)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
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