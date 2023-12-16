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

    // Map-based selection, this will matain pixels within the valid range
    // When there is no boundary, Min and Max are 0 and 1, respectively
    float gMinBorderPadding;        // (kernelSize / 2) / (float)shadowSize
    float gMaxBorderPadding;        // 1.0f - (kernelSize / 2) / (float)shadowSize
    float gMagicPower;              // The exponent used to handle light leakage
    int   gVisualizeCascades;       // 1 - Visualize cascaded shadow using different color; 2 - draw scene

    float gCascadeBlendArea;        // Mixing region when overlapping between cascades
    float gPCFDepthBias;            // Shadow offset value
    int   gPCFBlurForLoopStart;     // Cycle initial value, PCF core of 5x5 starting from -2
    int   gPCFBlurForLoopEnd;       // Cycle end value, PCF core of 5x5 should be set to 3

    float  gTexelSize;              // The texel size of shadow map
    float3 gLightDir;               // Direction of light

    float  gLightBleedingReduction; // Leakage control item of VSM
    float  gEvsmPosExp;             // Positive exponent of EVSM
    float  gEvsmNegExp;             // Negative exponent of EVSM
    int    g16BitShadow;            // Is 16-bit shadow format

    float4 gCascadedFrustumsEyeSpaceDepthsDate[2];     // The z values of the far plane of different frustums, used to cascade
    static float gCascadedFrustumsEyeSpaceDepths[8] = (float[8])gCascadedFrustumsEyeSpaceDepthsDate;  // Doesn't belong to cbuffer, shaould not be accessed externally
}

#endif