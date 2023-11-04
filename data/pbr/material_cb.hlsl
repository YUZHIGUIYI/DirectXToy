#ifndef _MATERIAL_CB_
#define _MATERIAL_CB_

// Basic PBR material attribute sets
cbuffer CBMaterial : register(b1)
{
    // For geometry pass - pixel shader
    float4 gBaseColorOpacity;           // float3 color + float opacity
    float4 gSpecularAnisotropic;        // float3 specular color + float anisotropic

    float  gMetalness;          
    float  gRoughness;
    float  gSpecularStrength;
    float  gSpecularTint;
    
    uint   gNoDiffuseSrv;
    uint   gNoNormalSrv;
    uint   gNoMetalnessSrv;
    uint   gNoRoughnessSrv;
}

#endif 