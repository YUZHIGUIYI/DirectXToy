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

float3 clip_aabb(float3 aabb_min, float3 aabb_max, float3 prev_sample, float3 avg)
{
    const float eps = 0.000001f;

    float3 r = prev_sample - avg;
    float3 rmax = aabb_max - avg;
    float3 rmin = aabb_min - avg;

    if (r.x > rmax.x + eps) 
        r *= (rmax.x / r.x);
    if (r.y > rmax.y + eps)
        r *= (rmax.y / r.y);
    if (r.z > rmax.z + eps)
        r *= (rmax.z / r.z);

    if (r.x < rmin.x - eps)
        r *= (rmin.x / r.x);
    if (r.y < rmin.y - eps)
        r *= (rmin.y / r.y);
    if (r.z < rmin.z - eps)
        r *= (rmin.z / r.z);

    return avg + r;
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

    float temp = YCoCgR_color.x - YCoCgR_color.z / 2;
    rgb_color.g = YCoCgR_color.z + temp;
    rgb_color.b = temp - YCoCgR_color.y / 2;
    rgb_color.r = rgb_color.b + YCoCgR_color.y;

    return rgb_color;
}

//// Optimized HDR weighting function
float hdr_weight4(float3 color, float exposure)
{
    return rcp(color.r * exposure + 4.0f);
}

// Pixel shader
float4 PS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    int x, y, i;
    float closest_depth = gFarZ;  // ???
    float len_velocity = 0.0f;
    float2 velocity = float2(0.0f, 0.0f);
    float2 closest_offset = float2(0.0f, 0.0f);
    double2 jittered_uv = (double2)texcoord + gJitter.xy;
    
    for (y = -1; y <= 1; ++y)
    {
        for (x = -1; x <= 1; ++x)
        {
            float2 sample_offset = float2(x, y) * gInvRenderTargetSize;
            float2 sample_uv = jittered_uv + sample_offset;
            sample_uv = saturate(sample_uv);

            float neighborhood_depth_samp = gDepthMap.Sample(gSamLinearClamp, sample_uv).r;
            neighborhood_depth_samp = linear_depth(neighborhood_depth_samp);    // Handle reverse z

            if (neighborhood_depth_samp > closest_depth)
            {
                closest_depth = neighborhood_depth_samp;
                closest_offset = float2(x, y);
            }
        }
    }
    closest_offset *= gInvRenderTargetSize;
    velocity = gVelocityMap.Sample(gSamLinearClamp, jittered_uv + closest_offset).rg;
    len_velocity = length(velocity);

    float2 cur_sample_uv = lerp(texcoord, jittered_uv, 0.0f);
    float3 cur_color = gCurrentFrameMap.Sample(gSamLinearClamp, cur_sample_uv).rgb;
    cur_color = RGB_TO_YCoCgR(cur_color);

    float3 prev_color = gHistoryFrameMap.Sample(gSamLinearClamp, texcoord - velocity).rgb;
    prev_color = RGB_TO_YCoCgR(prev_color);

    uint N = 9;
    float3 history;
    float3 sum = float3(0.0f, 0.0f, 0.0f);
    float3 m1 = float3(0.0f, 0.0f, 0.0f);
    float3 m2 = float3(0.0f, 0.0f, 0.0f);
    float3 neighbor_min = float3(9999999.0f, 9999999.0f, 9999999.0f);
    float3 neighbor_max = float3(-99999999.0f, -99999999.0f, -99999999.0f);
    float  neighborhood_final_weight = 0.0f;
    float  total_weight = 0.0f;
    // 3 x 3 sampling
    for (y = -1; y <= 1; ++y)
    {
        for (x = -1; x <= 1; ++x)
        {
            i = (y + 1) * 3 + x + 1;
            float2 sample_offset = float2(x, y) * gInvRenderTargetSize;
            float2 sample_uv = cur_sample_uv + sample_offset;
            sample_uv = saturate(sample_uv);

            float3 neighborhood_samp = gCurrentFrameMap.Sample(gSamLinearClamp, sample_uv).rgb;
            neighborhood_samp = max(neighborhood_samp, float3(0.0f, 0.0f, 0.0f));
            neighborhood_samp = RGB_TO_YCoCgR(neighborhood_samp);

            neighbor_min = min(neighbor_min, neighborhood_samp);
            neighbor_max = max(neighbor_max, neighborhood_samp);
            neighborhood_final_weight = hdr_weight4(neighborhood_samp, s_exposure);
            m1 += neighborhood_samp;
            m2 += neighborhood_samp * neighborhood_samp;
            total_weight += neighborhood_final_weight;
            sum += neighborhood_samp * neighborhood_final_weight;
        }
    }
    float3 filtered = sum / total_weight;

    // Variance clip
    float3 mu = m1 / N;
    float3 sigma = sqrt(abs(m2 / N - mu * mu));
    float3 minc = mu - s_variance_clip_gamma * sigma;
    float3 maxc = mu + s_variance_clip_gamma * sigma;

    prev_color = clip_aabb(minc, maxc, prev_color, mu);

    float cur_weight = lerp(s_blend_weight_lower_bound, s_blend_weight_upper_bound, saturate(len_velocity * s_blend_weight_velocity_scale));
    float prev_weight = 1.0f - cur_weight;

    float rcp_weight = rcp(cur_weight + prev_weight);
    history = (cur_color * cur_weight + prev_color * prev_weight) * rcp_weight;
    history.rgb = YCoCgR_To_RGB(history.rgb);

    return float4(history, 1.0f);
}