#include "screen_triangle_vs.hlsl"
#include "samplers.hlsl"

// Textures
Texture2D gHistoryFrameMap : register(t0);

Texture2D gCurrentFrameMap : register(t1);

Texture2D gVelocityMap     : register(t2);

Texture2D gDepthMap        : register(t3);

// Constant buffer
cbuffer CBPerPass : register(b0)
{
    float  gNearZ;
    float  gFarZ;
    float2 gJitter;

    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;

    int    gFirstFrame;
    int3   gFirstFramePadding;
}

// Constants
static const float s_variance_clip_gamma = 1.0f;
static const float s_exposure = 10.0f;
static const float s_blend_weight_lower_bound = 0.03f;
static const float s_blend_weight_upper_bound = 0.12f;
static const float s_blend_weight_velocity_scale = 6000.0f;

// Helper functions
float linear_depth(float depth)
{
    return (depth * gNearZ) / (gFarZ - depth * (gFarZ - gNearZ));
}

float3 RGB_TO_YCoCgR(float3 rgb_color)
{
    float3 YCoCgR_color;

    YCoCgR_color.y = rgb_color.r - rgb_color.b;
    float temp = rgb_color.b + YCoCgR_color.y / 2.0f;
    YCoCgR_color.z = rgb_color.g - temp;
    YCoCgR_color.x = temp + YCoCgR_color.z / 2.0f;

    return YCoCgR_color;
}

float3 YCoCgR_To_RGB(float3 YCoCgR_color)
{
    float3 rgb_color;

    float temp = YCoCgR_color.x - YCoCgR_color.z / 2.0f;
    rgb_color.g = YCoCgR_color.z + temp;
    rgb_color.b = temp - YCoCgR_color.y / 2.0f;
    rgb_color.r = rgb_color.b + YCoCgR_color.y;

    return rgb_color;
}

// Tone mapping
float luminance(float3 color)
{
    return 0.25f * color.r + 0.5f * color.g + 0.25f * color.b;
}

float3 tonemap(float3 color)
{
    return color / (1.0f + luminance(color));
}

float3 untonemap(float3 color)
{
    return color / (1.0f - luminance(color));
}

// Modified clip aabb
float3 clip_aabb(float3 cur_color, float3 pre_color, float2 texcoord)
{
    float3 aabb_min = cur_color;
    float3 aabb_max = cur_color;

    float3 m1 = float3(0.0f, 0.0f, 0.0f);
    float3 m2 = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            float2 sample_uv = texcoord + float2(i, j) * gInvRenderTargetSize;
            sample_uv = saturate(sample_uv);
            float3 color = gCurrentFrameMap.Sample(gSamLinearWrap, sample_uv).rgb;
            color = RGB_TO_YCoCgR(tonemap(color));
            m1 += color;
            m2 += color * color;
        }
    }

    // Variance clip
    const int N = 9;
    const float variance_clip_gamma = 1.0f;
    float3 mu = m1 / N;
    float3 sigma = sqrt(abs(m2 / N - mu * mu));
    aabb_min = mu - variance_clip_gamma * sigma;
    aabb_max = mu + variance_clip_gamma * sigma;

    // Clip to center
    float3 p_clip = 0.5f * (aabb_max + aabb_min);
    float3 e_clip = 0.5f * (aabb_max - aabb_min);

    float3 v_clip = pre_color - p_clip;
    float3 v_unit = v_clip / e_clip;
    float3 a_unit = abs(v_unit);
    float  ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));

    if (ma_unit > 1.0f) 
    {
        return p_clip + v_clip / ma_unit;
    }
    else 
    {
        return pre_color;
    }
}

// Pixel shader
float4 PS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    int x, y, i;
    float closest_depth = gFarZ; 
    float len_velocity = 0.0f;
    float2 velocity = float2(0.0f, 0.0f);
    float2 closest_offset = float2(0.0f, 0.0f);
    // float2 jittered_uv = texcoord + gJitter.xy;
    float2 jittered_uv = texcoord;
    
    // 3 x 3 velocity
    for (y = -1; y <= 1; ++y)
    {
        for (x = -1; x <= 1; ++x)
        {
            float2 sample_offset = float2(x, y) * gInvRenderTargetSize;
            float2 sample_uv = jittered_uv + sample_offset;
            sample_uv = saturate(sample_uv);

            float neighborhood_depth_samp = gDepthMap.Sample(gSamPointClamp, sample_uv).r;
            neighborhood_depth_samp = linear_depth(neighborhood_depth_samp);

            if (neighborhood_depth_samp > closest_depth)
            {
                closest_depth = neighborhood_depth_samp;
                closest_offset = sample_offset;
            }
        }
    }
    velocity = gVelocityMap.Sample(gSamLinearWrap, jittered_uv + closest_offset).rg;
    len_velocity = length(velocity);

    float first_frame = (float)gFirstFrame;
    float2 cur_sample_uv = texcoord;
    float3 cur_color = gCurrentFrameMap.Sample(gSamLinearWrap, cur_sample_uv).rgb;

    float2 pre_sample_uv = saturate(texcoord - velocity);
    float3 prev_color = gHistoryFrameMap.Sample(gSamLinearWrap, pre_sample_uv).rgb;

    cur_color = RGB_TO_YCoCgR(tonemap(cur_color));
    prev_color = RGB_TO_YCoCgR(tonemap(prev_color));

    prev_color = clip_aabb(cur_color, prev_color, texcoord);

    prev_color = untonemap(YCoCgR_To_RGB(prev_color));
    cur_color = untonemap(YCoCgR_To_RGB(cur_color));

    // Handle first frame
    prev_color = lerp(prev_color, cur_color, float3(first_frame, first_frame, first_frame));

    const float weight = 0.05f;
    float3 final_color = weight * cur_color + (1.0f - weight) * prev_color;
    return float4(final_color, 1.0f);
}