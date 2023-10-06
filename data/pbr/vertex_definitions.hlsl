#ifndef _VERTEX_DEFINITIONS_
#define _VERTEX_DEFINITIONS_

struct VertexShaderInput
{
    float3 position  : POSITION;
    float3 normal    : NORMAL;
    float4 tangent   : TANGENT;
    float2 texcoord  : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 homog_position  : SV_POSITION;
    float3 local_position  : POSITIONL;
    float3 world_position  : POSITION;
    float3 world_normal    : NORMAL;
    float3 tangent         : TANGENT;
    float3 bi_normal       : TBNOUT;
    float2 texcoord        : TEXCOORD;

    // TODO: enable TAA
    float4 cur_vp_position : POSITION1;
    float4 pre_vp_position : POSITION2;
};

#endif