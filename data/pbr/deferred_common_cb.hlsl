#ifndef _COMMON_CB_
#define _COMMON_CB_

// TODO: seperate constant buffer
// Constant data that varies per frame
cbuffer CBPerObject      : register(b0)
{
    // 1. For geometry pass - vertex shader
    matrix gWorld;
    matrix gView;
    matrix gViewProj;

    matrix gPreWorld;
    matrix gPreViewProj; 
    matrix gUnjitteredViewProj;

    // 2. For deferred pbr pass - pixel shader
    float4 gEyeWorldPos;

    uint   gNoPreprocess;
    uint3  gPreprocessPadding;

    // 3. For deferred pbr pass, cascaded shadow map - pixel shader
    matrix gShadowView;
    float4 gCascadedOffset[8];      // The translation of ShadowPT matrix
    float4 gCascadedScale[8];       // The scale of ShadowPT matrix

    int    gVisualizeCascades;      // 1 - visualize cascaded shadows using different colors
                                    // 0 - draw scene
    int    gPCFBlurForLoopStart;    // Initial value of cycle, PCF core of 5x5, start from -2
    int    gPCFBlurForLoopEnd;      // End value of cycle, PCF core of 5x5, set as 3
    int    gPadding;

    // Map-based selection, this will matain pixels within the valid range
    // When there is no boundary, Min and Max are 0 and 1, respectively
    float gMinBorderPadding;        // (kernelSize / 2) / (float)shadowSize
    float gMaxBorderPadding;        // 1.0f - (kernelSize / 2) / (float)shadowSize
    float gShadowBias;              // Handle the offset values of shadow artifacts, which will be exacerbated by PCF
    float gCascadeBlendArea;        // Mixing region when overlapping between cascades

    float  gTexelSize;              // The texel size of shadow map
    float3 gLightDir;               // Direction of light

    float4 gCascadedFrustumsEyeSpaceDepthsFloat[2];     // The z values of the far plane of different frustums, used to cascade
    float4 gCascadedFrustumsEyeSpaceDepthsFloat4[8];    // Used to array traversal, the yzw components are useless
};

#endif