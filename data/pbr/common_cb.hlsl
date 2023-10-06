#ifndef _COMMON_CB_
#define _COMMON_CB_

// Constant data that varies per frame
cbuffer CBPerObject : register(b0)
{
    matrix gWorld;
    matrix gViewProj;
    matrix gWorldViewProj;

    // New add
    matrix gPreWorld;
    matrix gPreViewProj; 
    matrix gUnjitteredViewProj;

    float4 gEyeWorldPos;
    uint   gNoPreprocess;
};

#endif