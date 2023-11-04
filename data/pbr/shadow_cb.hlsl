#ifndef _SHADOW_CB_
#define _SHADOW_CB_

cbuffer CBPerObject : register(b0)
{
    matrix gWorldViewProj;
}

#endif