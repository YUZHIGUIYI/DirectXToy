#include "vertex_definitions.hlsl"
#include "material_cb.hlsl"
#include "samplers.hlsl"
#include "gbuffer_registers.hlsl"

struct GBuffer
{
    float4 AlbedoMetalness : SV_Target0;
    float4 NormalRoughness : SV_Target1;
    float4 WorldPosition   : SV_Target2;
};

GBuffer PS(VertexShaderOutput pin)
{
    GBuffer gbuffer;

    float3 normal_from_tex = gNoNormalSrv == 1 ? float3(0.0f, 0.0f, 1.0f) :
        2 * (gNormalMap.Sample(gSamAnisotropicWrap, pin.texcoord).rgb - 0.5f);
    // Construct TBN
    float3x3 TBN = float3x3(pin.tangent, pin.bi_normal, pin.world_normal);
    float3 normal = mul(normal_from_tex, TBN);

    if (gNoDiffuseSrv == 1)
    {
        gbuffer.AlbedoMetalness = float4(1.0f, 1.0f, 1.0f, 1.0f) * gBaseColorOpacity;
    } else 
    {
        gbuffer.AlbedoMetalness = float4(gAlbedoMap.Sample(gSamAnisotropicWrap, pin.texcoord).rgb, 1.0f);
    }

    if (gNoMetalnessSrv == 1)
    {
        gbuffer.AlbedoMetalness.a = gMetalness;
    } else
    {
        gbuffer.AlbedoMetalness.a = gMetalnessMap.Sample(gSamAnisotropicWrap, pin.texcoord).r;
    }
    
    gbuffer.NormalRoughness = float4(normal, 1.0f);
    if (gNoRoughnessSrv == 1)
    {
        gbuffer.NormalRoughness.a = gRoughness;
    } else 
    {
        gbuffer.NormalRoughness.a = gRoughnessMap.Sample(gSamAnisotropicWrap, pin.texcoord).r;
    }

    gbuffer.WorldPosition = float4(pin.world_position, 1.0f);

    return gbuffer;
}

