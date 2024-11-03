#ifndef _SKY_
#define _SKY_

#define POST_COLOR_A 2.51f
#define POST_COLOR_B 0.03f
#define POST_COLOR_C 2.43f
#define POST_COLOR_D 0.59f
#define POST_COLOR_E 0.14f
#define PI 3.14159265f

cbuffer CBPSParams : register(b0)
{
    float3 gFrustumA;
    float  gPaddingA;

    float3 gFrustumB;
    float  gPaddingB;

    float3 gFrustumC;
    float  gPaddingC;

    float3 gFrustumD;
    float  gPaddingD;
}

Texture2D<float4> gSkyLutMap : register(t0);
SamplerState     gSamSkyView : register(s0);

struct VertexShaderOutput
{
    float4 homog_position : SV_POSITION;
    float2 texcoord       : TEXCOORD;
};

float3 tone_mapping(float3 input)
{
    return (input * (POST_COLOR_A * input + POST_COLOR_B)) /
            (input * (POST_COLOR_C * input + POST_COLOR_D) + POST_COLOR_E);
}

float3 post_process_color(float2 seed, float3 input)
{
    float3 output = tone_mapping(input);
    float rand = frac(sin(dot(seed, float2(12.9898f, 78.233f) * 2.0f)) * 43758.5453f);
    output = 255.0f * saturate(pow(output, 1.0f / 2.2f));
    float3 decimal_part = output - floor(output);
    output = rand.xxx < decimal_part ? ceil(output) : floor(output);

    return output / 255.0f;
}


// Use a triangle to cover the NDC space
// (-1, 1)________ (3, 1)
//        |   |  /
// (-1,-1)|___|/ (1, -1)   
//        |  /
// (-1,-3)|/    
VertexShaderOutput VS(uint vertex_id : SV_VertexID)
{
    VertexShaderOutput vs_output;
    float2 grid = float2((vertex_id << 1) & 2, vertex_id & 2);
    float2 xy = grid * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    vs_output.texcoord = grid * float2(1.0f, 1.0f);
    vs_output.homog_position = float4(xy, 0.0f, 1.0f);
    return vs_output;
}

float4 PS(VertexShaderOutput ps_input) : SV_Target0
{
    float3 dir = normalize(lerp(
        lerp(gFrustumA, gFrustumB, ps_input.texcoord.x),
        lerp(gFrustumC, gFrustumD, ps_input.texcoord.x), ps_input.texcoord.y));

    float phi = atan2(dir.z, dir.x);
    float u = phi / (2.0f * PI);

    float theta = asin(dir.y);
    float v = 0.5f + 0.5f * sign(theta) * sqrt(abs(theta) / (PI / 2.0f));

    float3 sky_color = gSkyLutMap.SampleLevel(gSamSkyView, float2(u, v), 0.0f);
    sky_color = post_process_color(ps_input.texcoord, sky_color);
    return float4(sky_color, 1.0f);
}

#endif