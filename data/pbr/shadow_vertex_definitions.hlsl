#ifndef _SHADOW_VERTEX_DEFINITIONS_
#define _SHADOW_VERTEX_DEFINITIONS_

struct VertexShaderInput
{
    float3 position       : POSITION;
    float3 normal         : NORMAL;
    float2 texcoord       : TEXCOORD;
};

struct VertexShaderOutput
{
    float4 homog_position : SV_POSITION;
    float2 texcoord       : TEXCOORD;
};

#endif