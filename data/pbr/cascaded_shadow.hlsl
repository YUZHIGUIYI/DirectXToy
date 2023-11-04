#ifndef _CASCADED_SHADOW_
#define _CASCADED_SHADOW_

#include "deferred_common_cb.hlsl"
#include "deferred_registers.hlsl"
#include "samplers.hlsl"

// 使用偏导，将shadow map中的texels映射到正在渲染的图元的观察空间平面上
// 该深度将会用于比较并减少阴影走样
// 这项技术是开销昂贵的，且假定对象是平面较多的时候才有效
#ifndef USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG
#define USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG 0
#endif

// 允许在不同级联之间对阴影值混合。当shadow maps比较小
// 且artifacts在两个级联之间可见的时候最为有效
#ifndef BLEND_BETWEEN_CASCADE_LAYERS_FLAG
#define BLEND_BETWEEN_CASCADE_LAYERS_FLAG 0
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

// 在大多数情况下，使用3-4个级联，并开启BLEND_BETWEEN_CASCADE_LAYERS_FLAG，
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

//--------------------------------------------------------------------------------------
// 为阴影空间的texels计算对应光照空间
//--------------------------------------------------------------------------------------
void calculate_right_and_up_texel_depth_deltas(float3 shadow_texel_ddx, float3 shadow_texel_ddy,
                                                out float up_texel_depth_weight, 
                                                out float right_texel_depth_weight)
{
    // 这里使用X和Y中的偏导数来计算变换矩阵。我们需要逆矩阵将我们从阴影空间变换到屏幕空间，
    // 因为这些导数能让我们从屏幕空间变换到阴影空间。新的矩阵允许我们从阴影图的texels映射
    // 到屏幕空间。这将允许我们寻找对应深度像素的屏幕空间深度。
    // 这不是一个完美的解决方案，因为它假定场景中的几何体是一个平面。

    // 在大多数情况下，使用偏移或方差阴影贴图是一种更好的、能够减少伪影的方法

    float2x2 mat_screen_to_shadow = float2x2(shadow_texel_ddx.xy, shadow_texel_ddy.xy);
    float det = determinant(mat_screen_to_shadow);
    float inv_det = 1.0f / det;
    float2x2 mat_shadow_to_screen = float2x2(
        mat_screen_to_shadow._22 * inv_det,  mat_screen_to_shadow._12 * -inv_det,
        mat_screen_to_shadow._21 * -inv_det, mat_screen_to_shadow._11 * inv_det
    );

    float2 right_shadow_texel_location = float2(gTexelSize, 0.0f);
    float2 up_shadow_texel_location = float2(0.0f, gTexelSize);

    // 通过阴影空间到屏幕空间的矩阵变换右边的texel
    float2 right_texel_depth_ratio = mul(right_shadow_texel_location, mat_shadow_to_screen);
    float2 up_texel_depth_ratio = mul(up_shadow_texel_location, mat_shadow_to_screen);

    // 现在可以计算在shadow map向右和向上移动时，深度的变化值
    // 使用x方向和y方向变换的比值乘上屏幕空间x和y深度的导数来计算变化值
    up_texel_depth_weight = up_texel_depth_ratio.x * shadow_texel_ddx.z + up_texel_depth_ratio.y * shadow_texel_ddy.z;
    right_texel_depth_weight = right_texel_depth_ratio.x * shadow_texel_ddx.z + right_texel_depth_ratio.y * shadow_texel_ddy.z;
}

//--------------------------------------------------------------------------------------
// 使用PCF采样深度图并返回着色百分比
//--------------------------------------------------------------------------------------
float calculate_pcf_percent_lit(int current_cascade_index, float4 shadow_texel_coord, 
                                float right_texel_depth_delta, float up_texel_depth_delta,
                                float blur_size)
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
            depth_cmp -= gShadowBias;
            if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
            {
                depth_cmp += right_texel_depth_delta * (float)x + up_texel_depth_delta * (float)y;
            }
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
    float blend_interval = gCascadedFrustumsEyeSpaceDepthsFloat4[current_cascade_index].x;

    if (current_cascade_index > 0)
    {
        int blend_interval_below_index = current_cascade_index - 1;
        pixel_depth -= gCascadedFrustumsEyeSpaceDepthsFloat4[blend_interval_below_index].x;
        blend_interval -= gCascadedFrustumsEyeSpaceDepthsFloat4[blend_interval_below_index].x;
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
    // blend_band_location = min(tx, ty, 1.0 - tx, 1.0 - ty);
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
            float4 cmp_vec1 = (current_pixel_depth_vec > gCascadedFrustumsEyeSpaceDepthsFloat[0]);
            float4 cmp_vec2 = (current_pixel_depth_vec > gCascadedFrustumsEyeSpaceDepthsFloat[1]);
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
    float3 shadow_map_texel_coord_ddx;
    float3 shadow_map_texel_coord_ddy;
    // 这些偏导用于计算投影纹理空间相邻texel对应到光照空间不同方向引起的深度变化
    if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
    {
        // 计算光照空间的偏导映射到投影纹理空间的变化率
        shadow_map_texel_coord_ddx = ddx(shadow_map_texel_coord_view_space);
        shadow_map_texel_coord_ddy = ddy(shadow_map_texel_coord_view_space);

        shadow_map_texel_coord_ddx *= gCascadedScale[current_cascade_index];
        shadow_map_texel_coord_ddy *= gCascadedScale[current_cascade_index];

        calculate_right_and_up_texel_depth_deltas(shadow_map_texel_coord_ddx, shadow_map_texel_coord_ddy,
                                                up_texel_depth_weight, right_texel_depth_weight);
    }

    visualize_cascade_color = s_cascaded_color_multiplier[current_cascade_index];

    percent_lit = calculate_pcf_percent_lit(current_cascade_index, shadow_map_texel_coord,
                                            right_texel_depth_weight, up_texel_depth_weight, blur_size);

    //
    // 在两个级联之间进行混合
    //
    if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
    {
        // 为下一个级联重复进行投影纹理坐标的计算
        // 下一级联的索引用于在两个级联之间模糊
        next_cascade_index = min(CASCADE_COUNT_FLAG - 1, current_cascade_index + 1);
    }

    blend_between_cascades_amount = 1.0f;
    float current_pixels_blend_band_location = 1.0f;
    if (SELECT_CASCADE_BY_INTERVAL_FLAG)
    {
        if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
        {
            calculate_blend_amount_for_interval(current_cascade_index, current_pixel_depth,
                                                current_pixels_blend_band_location, blend_between_cascades_amount);
        }
    } else 
    {
        if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG)
        {
            calculate_blend_amount_for_map(shadow_map_texel_coord, 
                                            current_pixels_blend_band_location, blend_between_cascades_amount);
        }
    }

    if (BLEND_BETWEEN_CASCADE_LAYERS_FLAG && CASCADE_COUNT_FLAG > 1)
    {
        if (current_pixels_blend_band_location < gCascadeBlendArea)
        {
            // 计算下一级联的投影纹理坐标
            shadow_map_texel_coord_blend = shadow_map_texel_coord_view_space * gCascadedScale[next_cascade_index] + gCascadedOffset[next_cascade_index];

            // 在级联之间混合时，为下一级联进行计算
            if (current_pixels_blend_band_location < gCascadeBlendArea)
            {
                // 当前像素在混合地带内
                if (USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG)
                {
                    calculate_right_and_up_texel_depth_deltas(shadow_map_texel_coord_ddx, shadow_map_texel_coord_ddy,
                                                            up_texel_depth_weight_blend, right_texel_depth_weight_blend);
                }
                percent_lit_blend = calculate_pcf_percent_lit(next_cascade_index, shadow_map_texel_coord_blend,
                                                            right_texel_depth_weight_blend, up_texel_depth_weight_blend, blur_size);
                // 对两个级联的PCF混合
                percent_lit = lerp(percent_lit_blend, percent_lit, blend_between_cascades_amount);
            }
        }
    }

    return percent_lit;
}

#endif
