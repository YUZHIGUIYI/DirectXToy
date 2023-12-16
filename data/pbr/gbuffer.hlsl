#include "vertex_definitions.hlsl"
#include "material_cb.hlsl"
#include "samplers.hlsl"
#include "gbuffer_registers.hlsl"

struct GBuffer
{
    float4 AlbedoMetalness : SV_Target0;
    float4 NormalRoughness : SV_Target1;
    float4 WorldPosition   : SV_Target2;
    float2 MotionVector    : SV_Target3;
    uint   EntityID        : SV_Target4;
};

static const float3 s_normal = float3(0.0f, 0.0f, 1.0f);

GBuffer PS(VertexShaderOutput pin)
{
    GBuffer gbuffer;

    float3 normal_from_tex = lerp(2.0f * (gNormalMap.Sample(gSamAnisotropicWrap, pin.texcoord).rgb - 0.5f), s_normal, gNoNormalSrv);
    // Construct TBN
    float3x3 TBN = float3x3(pin.tangent, pin.bi_normal, pin.world_normal);
    float3 normal = mul(normal_from_tex, TBN);

    float3 albedo = gAlbedoMap.Sample(gSamAnisotropicWrap, pin.texcoord).rgb;
    gbuffer.AlbedoMetalness.rgb = lerp(albedo, gBaseColorOpacity.rgb, gNoDiffuseSrv);
    gbuffer.AlbedoMetalness.a = lerp(gMetalnessMap.Sample(gSamAnisotropicWrap, pin.texcoord).r, gMetalness, gNoMetalnessSrv);
    
    gbuffer.NormalRoughness.rgb = normal;
    gbuffer.NormalRoughness.a = lerp(gRoughnessMap.Sample(gSamAnisotropicWrap, pin.texcoord).r, gRoughness, gNoRoughnessSrv);

    gbuffer.WorldPosition = float4(pin.world_position, 1.0f);

    // TAA - motion vector
    float4 pre_vp_pos = pin.pre_vp_position;
    float4 cur_vp_pos = pin.cur_vp_position;
    pre_vp_pos = pre_vp_pos / pre_vp_pos.w;
    cur_vp_pos = cur_vp_pos / cur_vp_pos.w;
    //// Negate Y because world coordinate and texture coordinate have different Y axis
    pre_vp_pos.xy = pre_vp_pos.xy / float2(2.0f, -2.0f) + float2(0.5f, 0.5f);
    cur_vp_pos.xy = cur_vp_pos.xy / float2(2.0f, -2.0f) + float2(0.5f, 0.5f);

    gbuffer.MotionVector = float2(cur_vp_pos.x - pre_vp_pos.x, cur_vp_pos.y - pre_vp_pos.y);

    gbuffer.EntityID = pin.entity_id;

    return gbuffer;
}

