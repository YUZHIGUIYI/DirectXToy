#ifndef _SHADOW_PS_
#define _SHADOW_PS_

#include "shadow_vertex_definitions.hlsl"
#include "shadow_cb.hlsl"
#include "samplers.hlsl"
#include "screen_triangle_vs.hlsl"

#ifndef BLUR_KERNEL_SIZE
#define BLUR_KERNEL_SIZE 9
#endif

Texture2D gShadowMap : register(t0);

static const int BLUR_KERNEL_BEGIN        = BLUR_KERNEL_SIZE / -2;
static const int BLUR_KERNEL_END          = BLUR_KERNEL_SIZE /  2 + 1;
static const float FLOAT_BLUR_KERNEL_SIZE = (float)BLUR_KERNEL_SIZE;

float2 get_evsm_exponents(in float positive_exponent, in float negative_exponent, in int is_16bit_format)
{
    const float max_exponent = (is_16bit_format ? 5.54f : 42.0f);
    float2 exponents = float2(positive_exponent, negative_exponent);

    // Clamp exponent range to prevent overflow
    return min(exponents, max_exponent);
}

// Note: the input depth needs to be within the range of [0, 1]
float2 apply_evsm_exponents(float depth, float2 exponents)
{
    depth = 2.0f * depth - 1.0f;
    float2 exp_depth;
    exp_depth.x = exp(exponents.x * depth);
    exp_depth.y = -exp(-exponents.y * depth);

    return exp_depth;
}

float ShadowPS(VertexShaderOutput pin) : SV_Target
{
    return pin.homog_position.z;
}

float2 VarianceShadowPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    uint2 coords = uint2(homog_position.xy);

    float2 depth;
    depth.x = gShadowMap[coords];
    depth.y = depth.x * depth.x;

    return depth;
}

float ExponentialShadowPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD, uniform float c) : SV_Target
{
    uint2 coords = uint2(homog_position.xy);
    return c * gShadowMap[coords];
}

float2 EVSM2CompPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    uint2 coords = uint2(homog_position.xy);

    float2 exponents = get_evsm_exponents(gEvsmExponents.x, gEvsmExponents.y, g16BitShadow);
    float2 depth = apply_evsm_exponents(gShadowMap[coords].x, exponents);
    float2 out_depth = float2(depth.x, depth.x * depth.x);

    return out_depth;
}

float4 EVSM4CompPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    uint2 coords = uint2(homog_position.xy);

    float2 depth = apply_evsm_exponents(gShadowMap[coords].x, gEvsmExponents);
    float4 out_depth = float4(depth, depth * depth).xzyw;

    return out_depth;
}

float4 DebugShadowPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    uint2 coords = uint2(homog_position.xy);

    float depth = gShadowMap[coords].x;
    
    return float4(depth, depth, depth, 1.0f);
}

float4 GaussianBlurXPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 depths = 0.0f;

    [unroll]
    for (int x = BLUR_KERNEL_BEGIN; x < BLUR_KERNEL_END; ++x)
    {
        depths += gBlurWeights[x - BLUR_KERNEL_BEGIN] * gShadowMap.Sample(gSamPointClamp, texcoord, int2(x, 0));
    }

    return depths;
}

float4 GaussianBlurYPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 depths = 0.0f;

    [unroll]
    for (int y = BLUR_KERNEL_BEGIN; y < BLUR_KERNEL_END; ++y)
    {
        depths += gBlurWeights[y - BLUR_KERNEL_BEGIN] * gShadowMap.Sample(gSamPointClamp, texcoord, int2(0, y));
    }

    return depths;
}

float LogGaussianBlurPS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float cd0 = gShadowMap.Sample(gSamPointClamp, texcoord);
    float sum = gBlurWeights[FLOAT_BLUR_KERNEL_SIZE / 2] * gBlurWeights[FLOAT_BLUR_KERNEL_SIZE / 2];

    [unroll]
    for (int i = BLUR_KERNEL_BEGIN; i < BLUR_KERNEL_END; ++i)
    {
        for (int j = BLUR_KERNEL_BEGIN; j < BLUR_KERNEL_END; ++j)
        {
            float cdk = gShadowMap.Sample(gSamPointClamp, texcoord, int2(i, j)).x * (float) (i != 0 || j != 0);
            sum += gBlurWeights[i - BLUR_KERNEL_BEGIN] * gBlurWeights[j - BLUR_KERNEL_BEGIN] * exp(cdk - cd0);
        }
    }

    sum = log(sum) + cd0;
    sum = isinf(sum) ? 84.0f : sum;

    return sum;
}

#endif