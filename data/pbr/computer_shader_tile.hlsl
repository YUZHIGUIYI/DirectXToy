#ifndef _COMPUTER_SHADER_TILE_
#define _COMPUTER_SHADER_TILE_

#include "tile_info.hlsl"

cbuffer CBPerFrame : register(b0)
{
    matrix gProj;
    float4 gCameraNearFar;
    uint4  gFramebufferDimensions;
}

RWStructuredBuffer<TileInfo> gTileBufferRW : register(u0);

RWStructuredBuffer<uint2>    gFramebuffer  : register(u1);

Texture2D gDepthMap                        : register(t0);

// Light list in current tile
groupshared uint gsTileLightIndices[MAX_LIGHTS >> 3];
groupshared uint gsTileNumLights;
groupshared uint gsMinZ;
groupshared uint gsMaxZ;

// Calculate frustum planes for each tile
void construct_frustum_planes(uint3 group_id, float min_tile_z, float max_tile_z, out float4 frustum_planes[6])
{
    // Uniform tile 
    float2 tile_scale = float2(gFramebufferDimensions.xy) / COMPUTE_SHADER_TILE_GROUP_DIM;
    float2 tile_bias = tile_scale - float2(1.0f, 1.0f) - float2(2.0f, 2.0f) * float2(group_id.xy);

    // Calculate projection matrix for current tile frustum
    float4 c1 = float4(gProj._11 * tile_scale.x, 0.0f, tile_bias.x, 0.0f);
    float4 c2 = float4(0.0f, gProj._22 * tile_scale.y, -tile_bias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    // Gribb/Hartmann method to extract frustum planes
    frustum_planes[0] = c4 - c1;  // right plane
    frustum_planes[1] = c4 + c1;  // left plane
    frustum_planes[2] = c4 - c2;  // top plane
    frustum_planes[3] = c4 + c2;  // bottom plane
    frustum_planes[4] = float4(0.0f, 0.0f, 1.0f, -min_tile_z);  // near plane
    frustum_planes[5] = float4(0.0f, 0.0f, -1.0f, max_tile_z);  // far plane

    // Normalize
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustum_planes[i] *= rcp(length(frustum_planes[i].xyz));
    }
}

[numthreads(COMPUTE_SHADER_TILE_GROUP_DIM, COMPUTE_SHADER_TILE_GROUP_DIM, 1)]
void ComputeShaderTileCS(uint3 group_id : SV_GroupID,
                            uint3 dispatch_thread_id : SV_DispatchThreadID,
                            uint3 group_thread_id : SV_GroupThreadID,
                            uint group_index : SV_GroupIndex)
{
    uint2 global_coords = dispatch_thread_id.xy;

    // TODO: consider reverse z
    float min_z_sample = gCameraNearFar.y;
    float max_z_sample = gCameraNearFar.x;
    {
        float z_buffer = gDepthMap.Load(int3(global_coords, 0));
        float view_space_z = gProj._m32 / (z_buffer - gProj._m22);

        // Avoid skybox
        bool valid_pixel = (view_space_z >= gCameraNearFar.x) && (view_space_z < gCameraNearFar.y);
        [flatten]
        if (valid_pixel)
        {
            min_z_sample = min(min_z_sample, view_space_z);
            max_z_sample = max(max_z_sample, view_space_z);
        }
    }
    
    // Initialize shadered memory
    if (group_index == 0)
    {
        gsTileNumLights = 0;
        gsMinZ = 0x7F7FFFFF;  // Maximum float 
        gsMaxZ = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    if (max_z_sample >= min_z_sample)
    {
        InterlockedMin(gsMinZ, asuint(min_z_sample));
        InterlockedMax(gsMaxZ, asuint(max_z_sample));
    }

    GroupMemoryBarrierWithGroupSync();

    float min_tile_z = asfloat(gsMinZ);
    float max_tile_z = asfloat(gsMaxZ);
    float4 frustum_planes[6];
    construct_frustum_planes(group_id, min_tile_z, max_tile_z, frustum_planes);

    // Cull current tile
    uint total_lights, dummy;
    gLights.GetDimensions(total_lights, dummy);

    // Calculate position
    uint2 dispatch_width = (gFramebufferDimensions.x + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
    uint tilebuffer_index = group_id.y * dispatch_width + group_id.x;

    [loop]
    for (uint light_index = group_index; light_index < total_lights; light_index += COMPUTE_SHADER_TILE_GROUP_SIZE)
    {
        PointLight light = gLights[light_index];

        // Bounding 
        bool in_frustum = true;
        [unroll]
        for (uint i = 0; i < 6; ++i)
        {
            float d = dot(frustum_planes[i], float4(light.view_position, 1.0f));
            in_frustum = in_frustum && (d >= -light.attenuation_end);
        }

        [branch]
        if (in_frustum)
        {
            // Append to list
            uint list_index;
            InterlockedAdd(gsTileNumLights, 1, list_index);
            gTileBufferRW[tilebuffer_index].tile_light_indices[list_index] = light_index;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    if (group_index == 0)
    {
        gTileBufferRW[tilebuffer_index].tile_num_lights = gsTileNumLights;
    }
}

#endif