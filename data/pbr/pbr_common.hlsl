#ifndef _PBR_COMMON_
#define _PBR_COMMON_

struct PBRMaterial
{
    float3 base_color;
    float  opacity;

    float3 specular_color;
    float  anisotropic;

    float  metalness;          
    float  roughness;
    float  specular_strength;
    float  specular_tint;
};

#endif