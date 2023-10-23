#ifndef _CASCADED_SHADOW_
#define _CASCADED_SHADOW_

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
void calculate_right_and_up_texel_depth_deltas(float3 shadow_tex_ddx, float3 shadow_tex_ddy,
                                                out float up_tex_depth_weight, 
                                                out float right_tex_depth_weight)
{
    // 这里使用X和Y中的偏导数来计算变换矩阵。我们需要逆矩阵将我们从阴影空间变换到屏幕空间，
    // 因为这些导数能让我们从屏幕空间变换到阴影空间。新的矩阵允许我们从阴影图的texels映射
    // 到屏幕空间。这将允许我们寻找对应深度像素的屏幕空间深度。
    // 这不是一个完美的解决方案，因为它假定场景中的几何体是一个平面。

    // 在大多数情况下，使用偏移或方差阴影贴图是一种更好的、能够减少伪影的方法

    float2x2 mat_screen_to_shadow = float2x2(shadow_tex_ddx.xy, shadow_tex_ddy.xy);
    float det = determinant(mat_screen_to_shadow);
    float inv_det = 1.0f / det;
    float2x2 mat_shadow_to_screen = float2x2(
        mat_screen_to_shadow._22 * inv_det, mat_screen_to_shadow._12 * -inv_det,
        mat_screen_to_shadow._21 * -inv_det, mat_screen_to_shadow._11 * inv_det
    );

}

#endif
