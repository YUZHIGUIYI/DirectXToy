#ifndef _SHADOW_CB_
#define _SHADOW_CB_

cbuffer CBPerObject : register(b0)
{
    matrix gWorldViewProj;
}

// cbuffer CBCascadedShadow           : register(b1)
// {
//     matrix gShadowView;
//     float4 gCascadedOffset[8];      // The translation of ShadowPT matrix
//     float4 gCascadedScale[8];       // The scale of ShadowPT matrix

//     int    gVisualizeCascades;      // 1 - visualize cascaded shadows using different colors
//                                     // 0 - draw scene
//     int    gPCFBlurForLoopStart;    // Initial value of cycle, PCF core of 5x5, start from -2
//     int    gPCFBlurForLoopEnd;      // End value of cycle, PCF core of 5x5, set as 3
//     int    gPadding;

//     float  gTexelSize;              // The texel size of shadow map
//     float3 gLightDir;               // Direction of light

//     float4 gCascadedFrustumsEyeSpaceDepthsFloat[2];     // The z values of the far plane of different frustums, used to cascade

//     float4 gCascadedFrustumsEyeSpaceDepthsFloat4[8];    // Used to array traversal, the yzw components are useless
// }

#endif