#include "vertex_definitions.hlsl"
#include "common_cb.hlsl"

VertexShaderOutput VS(VertexShaderInput vin)
{
    VertexShaderOutput vout;

    // Transform to world space
    float4 world_position = mul(float4(vin.position, 1.0f), gWorld);
    vout.world_position = world_position.xyz;
    vout.local_position = vin.position;

    // TODO: Enable TAA
    float4 pre_world_position = mul(float4(vin.position, 1.0f), gPreWorld);
    vout.cur_vp_position = mul(world_position, gUnjitteredViewProj);
    vout.pre_vp_position = mul(pre_world_position, gPreViewProj);

    // Assume non-uniform scaling, otherwise need to use inverse-transpose of world matrix
    vout.world_normal = normalize(mul(vin.normal, (float3x3)gWorld));

    // Transform to homogeneous clip space
    vout.homog_position = mul(world_position, gViewProj);

    // TODO: Consider texcoord offset
    // Output vertex attributes for interpolation across triangle - TAA
    vout.texcoord = vin.texcoord;

    vout.tangent = normalize(mul(vin.tangent, gWorld)).xyz;
    vout.bi_normal = cross(vout.world_normal, vout.tangent);

    return vout;
}