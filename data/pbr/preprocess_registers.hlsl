#ifndef _PREPROCESS_REGISTERS_
#define _PREPROCESS_REGISTERS_

// For equirect_to_cube.hlsl - input shader resource view from hdr image,
//                             only 2D-texture
Texture2D gInputHDRMap          : register(t0); 

// For sp_env_map.hlsl - input shader resource view from texture cube,
//                       this shader resource view would never be changed on CPU side,
//                       varying mip slice(computed from roughness in this shader), 0 first array slice, 6 array size
// For irradiance_map.hlsl - input shader resource view from texture cube, all subresources
TextureCube gInputCubeMap       : register(t1);

// For sp_brdf.hlsl
RWTexture2D<float2> gOutputLUT  : register(u0);

// For equirect_to_cube.hlsl - outPut unordered access view from texture cube, 
//                             0 mip slice, 0 first array slice, 6 array size
// For sp_env_map.hlsl - output unordered access view from environment map,
//                       variying mip slice(starting from 1 on CPU side), 0 first array slice, 6 array size
// For irradiance_map.hlsl - output unordered access view from irradiance map, 0 mip slice, 0 first array slice, 6 array size
RWTexture2DArray<float4> gOutputCubeMap : register(u1);

#endif