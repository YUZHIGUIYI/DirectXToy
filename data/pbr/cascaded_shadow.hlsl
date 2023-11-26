#ifndef _CASCADED_SHADOW_
#define _CASCADED_SHADOW_

#include "deferred_common_cb.hlsl"
#include "deferred_registers.hlsl"
#include "samplers.hlsl"

// 0 - Cascaded shadow map
// 1 - Variance shadow map
// 2 - Exponential shadow map
// 3 - Exponential variance shadow map 2-component
// 4 - Exponential variance shadow map 4-component
#ifndef SHADOW_TYPE
#define SHADOW_TYPE 4
#endif

// 有两种方法为当前像素片元选择合适的级联：
// Interval-based Selection 将视锥体的深度分区与像素片元的深度进行比较
// Map-based Selection 找到纹理坐标在shadow map范围中的最小级联
#ifndef SELECT_CASCADE_BY_INTERVAL_FLAG
#define SELECT_CASCADE_BY_INTERVAL_FLAG 0
#endif

// 级联数目
#ifndef CASCADE_COUNT_FLAG
#define CASCADE_COUNT_FLAG 3
#endif

// 在大多数情况下，使用3-4个级联，
// 可以适用于低端PC。高端PC可以处理更大的阴影，以及更大的混合地带
// 在使用更大的PCF核时，可以给高端PC使用基于偏导的深度偏移

static const float4 s_cascaded_color_multiplier[8] = 
{
    float4(1.5f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 1.5f, 0.0f, 1.0f),
    float4(0.0f, 0.0f, 5.5f, 1.0f),
    float4(1.5f, 0.0f, 5.5f, 1.0f),
    float4(1.5f, 1.5f, 0.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f, 1.0f),
    float4(0.0f, 1.0f, 5.5f, 1.0f),
    float4(0.5f, 3.5f, 0.75f, 1.0f)
};

float linear_step(float a, float b, float v)
{
    return saturate((v - a) / (b - a));
}

// 令 [0, amount] 的部分归零并将 (amount, 1] 重新映射到 (0, 1]
float reduce_light_bleeding(float p_max, float amount)
{
    return linear_step(amount, 1.0f, p_max);
}

float2 get_evsm_exponents(float positive_exponent, float negative_exponent, int is_16bit_shadow)
{
    const float max_exponent = (is_16bit_shadow ? 5.54f : 42.0f);

    float2 exponents = float2(positive_exponent, negative_exponent);

    // 限制指数范围，防止溢出
    return min(exponents, max_exponent);
}

// 要求输入的depth在 [0, 1] 范围内
float2 apply_evsm_exponents(float depth, float2 exponents)
{
    depth = 2.0f * depth - 1.0f;
    float2 exp_depth;
    exp_depth.x = exp(exponents.x * depth);
    exp_depth.y = -exp(-exponents.y * depth);

    return exp_depth;
}

float chebyshev_upper_bound(float2 moments, float receiver_depth, float min_variance, float light_bleeding_reduction)
{
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, min_variance);

    float d = receiver_depth - moments.x;
    float p_max = variance / (variance + d * d);

    p_max = reduce_light_bleeding(p_max, light_bleeding_reduction);

    // 单边切比雪夫
    return (receiver_depth <= moments.x ? 1.0f : p_max);
}

//--------------------------------------------------------------------------------------
// 使用PCF采样深度图并返回着色百分比
//--------------------------------------------------------------------------------------
float calculate_pcf_percent_lit(int current_cascade_index, float4 shadow_texel_coord, float blur_size)
{
    float percent_lit = 0.0f;

    // 该循环可以展开，并且如果PCF大小固定，可以使用纹理即时偏移从而改善性能
    for (int x = gPCFBlurForLoopStart; x < gPCFBlurForLoopEnd; ++x)
    {
        for (int y = gPCFBlurForLoopStart; y < gPCFBlurForLoopEnd; ++y)
        {
            float depth_cmp = shadow_texel_coord.z;
            // 解决PCF深度偏移问题 - 使用一个偏移值
            // 过大的偏移会导致Peter-panning（阴影跑出物体）
            // 过小的偏移会导致阴影失真
            depth_cmp -= gPCFDepthBias;
            // 将变换后的像素深度同阴影图中的深度进行比较
            percent_lit += gShadowMap.SampleCmpLevelZero(gSamShadow, 
                            float3(
                                shadow_texel_coord.x + (float)x * gTexelSize,
                                shadow_texel_coord.y + (float)y * gTexelSize,
                                (float)current_cascade_index
                            ), 
                            depth_cmp);
        }
    }
    percent_lit /= blur_size;

    return percent_lit;
}

//--------------------------------------------------------------------------------------
// VSM：采样深度图并返回着色百分比
//--------------------------------------------------------------------------------------
float calculate_variance_shadow(int current_cascade_index, float4 shadow_texel_coord, float4 shadow_texel_coord_view_space)
{
    float percent_lit = 0.0f;
    float2 moments = float2(0.0f, 0.0f);

    // 为了将求导从动态控制流中拉出来，计算观察空间坐标的偏导
    // 从而得到投影纹理空间坐标的偏导
    float3 shadow_texel_coord_ddx = ddx(shadow_texel_coord_view_space).xyz;
    float3 shadow_texel_coord_ddy = ddy(shadow_texel_coord_view_space).xyz;
    shadow_texel_coord_ddx *= gCascadedScale[current_cascade_index].xyz;
    shadow_texel_coord_ddy *= gCascadedScale[current_cascade_index].xyz;

    moments += gShadowMap.SampleGrad(gSamAnisotropicClamp,
                            float3(shadow_texel_coord.xy, (float)current_cascade_index),
                            shadow_texel_coord_ddx.xy, shadow_texel_coord_ddy.xy).xy;

    percent_lit = chebyshev_upper_bound(moments, shadow_texel_coord.z, 0.00001f, gLightBleedingReduction);

    return percent_lit;
}

//--------------------------------------------------------------------------------------
// ESM：采样深度图并返回着色百分比
//--------------------------------------------------------------------------------------
float calculate_exponential_shadow(int current_cascade_index, float4 shadow_texel_coord, float4 shadow_texel_coord_view_space)
{
    float percent_lit = 0.0f;
    float occluder = 0.0f;

    float3 shadow_texel_coord_ddx = ddx(shadow_texel_coord_view_space).xyz;
    float3 shadow_texel_coord_ddy = ddy(shadow_texel_coord_view_space).xyz;
    shadow_texel_coord_ddx *= gCascadedScale[current_cascade_index].xyz;
    shadow_texel_coord_ddy *= gCascadedScale[current_cascade_index].xyz;

    occluder += gShadowMap.SampleGrad(gSamAnisotropicClamp, 
                                float3(shadow_texel_coord.xy, (float)current_cascade_index),
                                shadow_texel_coord_ddx.xy, shadow_texel_coord_ddy.xy).x;

    percent_lit = saturate(exp(occluder - gMagicPower * shadow_texel_coord.z));

    return percent_lit;
}

//--------------------------------------------------------------------------------------
// EVSM：采样深度图并返回着色百分比
//--------------------------------------------------------------------------------------
float calculate_exponential_variance_shadow(int current_cascade_index, float4 shadow_texel_coord, float4 shadow_texel_coord_view_space)
{
    float percent_lit = 0.0f;

    float2 exponents = get_evsm_exponents(gEvsmPosExp, gEvsmNegExp, g16BitShadow);
    float2 exp_depth = apply_evsm_exponents(shadow_texel_coord.z, exponents);
    float4 moments = 0.0f;

    float3 shadow_texel_coord_ddx = ddx(shadow_texel_coord_view_space).xyz;
    float3 shadow_texel_coord_ddy = ddy(shadow_texel_coord_view_space).xyz;
    shadow_texel_coord_ddx *= gCascadedScale[current_cascade_index].xyz;
    shadow_texel_coord_ddy *= gCascadedScale[current_cascade_index].xyz;

    moments += gShadowMap.SampleGrad(gSamAnisotropicClamp,
                            float3(shadow_texel_coord.xy, (float)current_cascade_index),
                            shadow_texel_coord_ddx.xy, shadow_texel_coord_ddy.xy);

    percent_lit = chebyshev_upper_bound(moments.xy, exp_depth.x, 0.00001f, gLightBleedingReduction);

    if (SHADOW_TYPE == 4)
    {   
        float neg = chebyshev_upper_bound(moments.zw, exp_depth.y, 0.00001f, gLightBleedingReduction);
        percent_lit = min(percent_lit, neg);
    }

    return percent_lit;
}

//--------------------------------------------------------------------------------------
// 计算两个级联之间的混合量 及 混合将会发生的区域
//--------------------------------------------------------------------------------------
void calculate_blend_amount_for_interval(int current_cascade_index, inout float pixel_depth,
                                        inout float current_pixels_blend_band_location,
                                        out float blend_between_cascades_amount)
{
    //                  pixel_depth
    //           |<-      ->|
    // /-+-------/----------+------/--------
    // 0 N     F[0]               F[i]
    //          |<-blend_interval->|
    // blend_band_location = 1.0 - depth / F[0] or
    // blend_band_location = 1.0 - (depth - F[0]) / (F[i] - F[0])
    // blend_band_location [0, gCascadeBlendArea] 时，进行[0, 1]的过渡

    // 需要计算当前shadow map的边缘地带，在哪里将会淡化到下一个级联
    // 然后就可以提前脱离开销昂贵的PCF for循环
    float blend_interval = gCascadedFrustumsEyeSpaceDepths[current_cascade_index];

    if (current_cascade_index > 0)
    {
        int blend_interval_below_index = current_cascade_index - 1;
        pixel_depth -= gCascadedFrustumsEyeSpaceDepths[blend_interval_below_index];
        blend_interval -= gCascadedFrustumsEyeSpaceDepths[blend_interval_below_index];
    }

    // 当前像素的混合地带的位置
    current_pixels_blend_band_location = 1.0f - pixel_depth / blend_interval;
    // blend_between_cascades_amount用于最终的阴影色插值
    blend_between_cascades_amount = current_pixels_blend_band_location / gCascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// 计算两个级联之间的混合量 及 混合将会发生的区域
//--------------------------------------------------------------------------------------
void calculate_blend_amount_for_map(float4 shadow_map_texel_coord,
                                    inout float current_pixels_blend_band_location,
                                    inout float blend_between_cascades_amount)
{
    //   _____________________
    //  |       map[i+1]      |
    //  |                     |
    //  |      0_______0      |
    //  |______| map[i]|______|
    //         |  0.5  |
    //         |_______|
    //         0       0
    // blend_band_location = min(tx, ty, 1.0f - tx, 1.0f - ty);
    // blend_band_location位于 [0, gCascadeBlendArea] 时，进行[0, 1]的过渡
    float2 distance_to_one = float2(1.0f - shadow_map_texel_coord.x, 1.0f - shadow_map_texel_coord.y);
    current_pixels_blend_band_location = min(shadow_map_texel_coord.x, shadow_map_texel_coord.y);
    float current_pixels_blend_location2 = min(distance_to_one.x, distance_to_one.y);
    current_pixels_blend_band_location = 
        min(current_pixels_blend_band_location, current_pixels_blend_location2);

    blend_between_cascades_amount = current_pixels_blend_band_location / gCascadeBlendArea;
}

//--------------------------------------------------------------------------------------
// 计算级联显示色或者对应的过渡色
//--------------------------------------------------------------------------------------
float4 get_cascade_color_multipler(int current_cascade_index, int next_cascade_index,
                                    float blend_between_cascades_amount)
{
    return lerp(s_cascaded_color_multiplier[next_cascade_index],
                s_cascaded_color_multiplier[current_cascade_index],
                blend_between_cascades_amount);
}

//--------------------------------------------------------------------------------------
// 计算级联阴影
//--------------------------------------------------------------------------------------
float calculate_cascaded_shadow(float4 shadow_map_texel_coord_view_space,
                                float current_pixel_depth,
                                out int current_cascade_index,
                                out int next_cascade_index,
                                out float blend_between_cascades_amount)
{
    float4 shadow_map_texel_coord = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 shadow_map_texel_coord_blend = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float4 visualize_cascade_color = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float percent_lit = 0.0f;
    float percent_lit_blend = 0.0f;

    float up_texel_depth_weight = 0.0f;
    float right_texel_depth_weight = 0.0f;
    float up_texel_depth_weight_blend = 0.0f;
    float right_texel_depth_weight_blend = 0.0f;

    float blur_size = gPCFBlurForLoopEnd - gPCFBlurForLoopStart;
    blur_size *= blur_size;

    int cascade_found = 0;
    next_cascade_index = 1;

    //
    // 确定级联，变换阴影纹理坐标
    //
    
    // 当视锥体是均匀划分 且 使用了Interval-based selection技术时
    // 可以不需要循环来查找
    // 在这种情况下current_pixel_depth可以用于正确的视锥体数组中查找
    // Interval-based selection
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        current_cascade_index = 0;
        //                               Depth
        // /-+-------/----------------/----+-------/----------/
        // 0 N     F[0]     ...      F[i]        F[i+1] ...   F
        // Depth > F[i] to F[0] => index = i+1
        if (CASCADE_COUNT_FLAG > 1)
        {
            float4 current_pixel_depth_vec = current_pixel_depth;
            float4 cmp_vec1 = (current_pixel_depth_vec > gCascadedFrustumsEyeSpaceDepthsDate[0]);
            float4 cmp_vec2 = (current_pixel_depth_vec > gCascadedFrustumsEyeSpaceDepthsDate[1]);
            float index = dot(float4(CASCADE_COUNT_FLAG > 0,
                                    CASCADE_COUNT_FLAG > 1,
                                    CASCADE_COUNT_FLAG > 2,
                                    CASCADE_COUNT_FLAG > 3), cmp_vec1) + 
                            dot(float4(CASCADE_COUNT_FLAG > 4,
                                        CASCADE_COUNT_FLAG > 5,
                                        CASCADE_COUNT_FLAG > 6,
                                        CASCADE_COUNT_FLAG > 7), cmp_vec2);
            index = min(index, CASCADE_COUNT_FLAG - 1);
            current_cascade_index = (int)index;
        }

        shadow_map_texel_coord = shadow_map_texel_coord_view_space * gCascadedScale[current_cascade_index] + gCascadedOffset[current_cascade_index];
    }

    // Map-based selection
    if (!SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        current_cascade_index = 0;
        if (CASCADE_COUNT_FLAG == 1)
        {
            shadow_map_texel_coord = shadow_map_texel_coord_view_space * gCascadedScale[0] + gCascadedOffset[0];
        }
        if (CASCADE_COUNT_FLAG > 1)
        {
            // 寻找最近的级联，使得纹理坐标位于纹理边界内
            // min_border < tx, ty < max_border
            for (int cascade_index = 0; cascade_index < CASCADE_COUNT_FLAG && cascade_found == 0; ++cascade_index)
            {
                shadow_map_texel_coord = shadow_map_texel_coord_view_space * gCascadedScale[cascade_index] + gCascadedOffset[cascade_index];
                if (min(shadow_map_texel_coord.x, shadow_map_texel_coord.y) > gMinBorderPadding && 
                    max(shadow_map_texel_coord.x, shadow_map_texel_coord.y) < gMaxBorderPadding)
                {
                    current_cascade_index = cascade_index;
                    cascade_found = 1;
                }
            }
        }
    }

    //
    // 计算当前级联的PCF
    // 

    visualize_cascade_color = s_cascaded_color_multiplier[current_cascade_index];

    if (SHADOW_TYPE == 0)
    {
        percent_lit = calculate_pcf_percent_lit(current_cascade_index, shadow_map_texel_coord, blur_size);
    }
    if (SHADOW_TYPE == 1)
    {
        percent_lit = calculate_variance_shadow(current_cascade_index, shadow_map_texel_coord, shadow_map_texel_coord_view_space);
    }
    if (SHADOW_TYPE == 2)
    {
        percent_lit = calculate_exponential_shadow(current_cascade_index, shadow_map_texel_coord, shadow_map_texel_coord_view_space);
    }
    if (SHADOW_TYPE >= 3)
    {
        percent_lit = calculate_exponential_variance_shadow(current_cascade_index, shadow_map_texel_coord, shadow_map_texel_coord_view_space);
    }

    //
    // 在两个级联之间进行混合
    //
    
    // 为下一个级联重复进行投影纹理坐标的计算
    // 下一级联的索引用于在两个级联之间模糊
    next_cascade_index = min(CASCADE_COUNT_FLAG - 1, current_cascade_index + 1);

    blend_between_cascades_amount = 1.0f;
    float current_pixels_blend_band_location = 1.0f;
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        if (CASCADE_COUNT_FLAG > 1)
        {
            calculate_blend_amount_for_interval(current_cascade_index, current_pixel_depth,
                                                current_pixels_blend_band_location, blend_between_cascades_amount);
        }
    } else 
    {
        if (CASCADE_COUNT_FLAG > 1)
        {
            calculate_blend_amount_for_map(shadow_map_texel_coord, 
                                            current_pixels_blend_band_location, blend_between_cascades_amount);
        }
    }

    if (CASCADE_COUNT_FLAG > 1)
    {
        if (current_pixels_blend_band_location < gCascadeBlendArea)
        {
            // 计算下一级联的投影纹理坐标
            shadow_map_texel_coord_blend = shadow_map_texel_coord_view_space * gCascadedScale[next_cascade_index] + gCascadedOffset[next_cascade_index];

            // 在级联之间混合时，为下一级联进行计算
            if (current_pixels_blend_band_location < gCascadeBlendArea)
            {
                if (SHADOW_TYPE == 0)
                {
                    percent_lit_blend = calculate_pcf_percent_lit(next_cascade_index, shadow_map_texel_coord_blend, blur_size);
                }
                if (SHADOW_TYPE == 1)
                {
                    percent_lit_blend = calculate_variance_shadow(next_cascade_index, shadow_map_texel_coord_blend, shadow_map_texel_coord_view_space);
                }
                if (SHADOW_TYPE == 2)
                {
                    percent_lit_blend = calculate_exponential_shadow(next_cascade_index, shadow_map_texel_coord_blend, shadow_map_texel_coord_view_space);
                }
                if (SHADOW_TYPE >= 3)
                {
                    percent_lit_blend = calculate_exponential_variance_shadow(next_cascade_index, shadow_map_texel_coord_blend, shadow_map_texel_coord_view_space);
                }

                // 对两个级联的PCF混合
                percent_lit = lerp(percent_lit_blend, percent_lit, blend_between_cascades_amount);
            }
        }
    }

    return percent_lit;
}

#endif
