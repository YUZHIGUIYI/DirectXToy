#include "samplers.hlsl"

TextureCube<float4> gSkyboxMap : register(t0);
Texture2D           gDepthMap  : register(t1);
Texture2D           gSceneMap  : register(t2);

cbuffer CBPerObject : register(b0)
{
    matrix gViewProj;
}

struct SkyboxInput
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct SkyboxOutput
{
    float4 view_position : SV_Position;
    float3 skybox_coord  : SKYBOX_COORD;
};

SkyboxOutput VS(SkyboxInput vin)
{
    SkyboxOutput vout;
    
    // Ensure skybox's depth is 0.0f, since we have reversed z
    vout.view_position = mul(float4(vin.position, 0.0f), gViewProj).xyww;
    vout.view_position.z = 0.0f;
    vout.skybox_coord  = vin.position;

    return vout;
}

float4 PS(SkyboxOutput pin) : SV_Target
{
    float3 lit = float3(0.0f, 0.0f, 0.0f);
    int2 coords = pin.view_position.xy;

    // float depth = gDepthMap.Load(int3(coords, 0)).r;
    // if (depth <= 0.0f)
    // {
    //     float3 skybox = gSkyboxMap.SampleLevel(gSamAnisotropicWrap, pin.skybox_coord, 0.0f).rgb;
    //     lit += skybox;
    // } else 
    // {
    //     lit += gSceneMap.Load(int3(coords, 0)).rgb;
    // }
    // lit += gSceneMap.Load(int3(coords, 0)).rgb;

    float3 skybox = gSkyboxMap.SampleLevel(gSamAnisotropicWrap, pin.skybox_coord, 0.0f).rgb;
    lit += skybox;
    
    return float4(lit, 1.0f);
}