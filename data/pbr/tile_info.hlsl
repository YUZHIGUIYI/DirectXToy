#ifndef _TILE_INFO_
#define _TILE_INFO_

#define MAX_LIGHTS_POWER 11
#define MAX_LIGHTS (1 << MAX_LIGHTS_POWER)
#define MAX_LIGHT_INDICES ((MAX_LIGHTS >> 3) - 1)

#define COMPUTE_SHADER_TILE_GROUP_DIM 16
#define COMPUTE_SHADER_TILE_GROUP_SIZE (COMPUTE_SHADER_TILE_GROUP_DIM*COMPUTE_SHADER_TILE_GROUP_DIM)

struct TileInfo
{
    uint tile_num_lights;
    uint tile_light_indices[MAX_LIGHT_INDICES];
};

struct PointLight
{
    float3 view_position;
    float  attenuation_begin;
    float3 color;
    float  attenuation_end;
};

StructuredBuffer<PointLight> gLights : register(t7);

// Pack two coordinates(<= 16 bits) into a single uint
uint pack_coords(uint2 coords)
{
    return coords.y << 16 | coords.x;
}

// Unpack a single uint to two coordinates(<= 16 bits)
uint2 unpack_coords(uint coords)
{
    return uint2(coords & 0xFFFF, coords >> 16);
}

#endif