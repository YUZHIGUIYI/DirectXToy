#ifndef _FULL_SCREEN_TRIANGLE_
#define _FULL_SCREEN_TRIANGLE_

// Use a triangle to cover the NDC space
// (-1, 1)________ (3, 1)
//        |   |  /
// (-1,-1)|___|/ (1, -1)   
//        |  /
// (-1,-3)|/    

void VS(uint vertex_id : SV_VertexID, 
        out float4 homog_position : SV_Position,
        out float2 texcoord : TEXCOORD)
{
    float2 grid = float2((vertex_id << 1) & 2, vertex_id & 2);
    float2 xy = grid * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    
    texcoord = grid * float2(1.0f, 1.0f);
    homog_position = float4(xy, 1.0f, 1.0f);
}

#endif
