#ifndef _SHADOW_CB_
#define _SHADOW_CB_

cbuffer CBTransform : register(b0)
{
    matrix gWorldViewProj;
    float2 gEvsmExponents;
    int    g16BitShadow;
}

cbuffer CBBlur      : register(b1)
{
    float4 gBlurWeightsArray[4];
    static float gBlurWeights[16] = (float[16])gBlurWeightsArray;
}


#endif