#include "screen_triangle_vs.hlsl"
#include "pbr_common.hlsl"
#include "cascaded_shadow.hlsl"

// Constant normal incidence Fresnel factor for all dielectrics
static const float3 s_fresnel = float3(0.04f, 0.04f, 0.04f);

// Shlick's approximation of the Fresnel factor.
float3 fresnel_schlick(float3 F0, float normal_dot_view, float roughness)
{
    return F0 + (1.0f - F0) * pow(1.0 - normal_dot_view, 5.0f);
}

// Return number of mipmap levels for specular IBL environment map
uint query_prefiltered_specular_map_levels()
{
    uint width, height, levels;
    gPrefilteredSpecularMap.GetDimensions(0, width, height, levels);
    return levels;
}

// Sample prefiltered specular reflection environment at correct mipmap level
float3 sample_prefiltered_color(float3 view_dir, float3 normal, float roughness)
{
    // float3 reflection_dir = reflect(-view_dir, normal); 
    float3 reflection_dir = 2.0f * max(0.0f, dot(normal, view_dir)) * normal - view_dir;
    uint levels = query_prefiltered_specular_map_levels();
    float3 prefiltered_color = gPrefilteredSpecularMap.SampleLevel(gSamAnisotropicWrap, reflection_dir, levels * roughness).rgb;
    return prefiltered_color;
}

// Ambient BRDF
float3 ambient_brdf(PBRMaterial pbr_mat, float3 irradiance_color, float3 prefiltered_color, float2 BRDFLUT, float normal_dot_view)
{
    float3 F0 = lerp(s_fresnel, pbr_mat.base_color, pbr_mat.metalness);
    float3 KS = fresnel_schlick(F0, normal_dot_view, pbr_mat.roughness);
    float3 KD = lerp(float3(1.0f, 1.0f, 1.0f) - KS, float3(0.0f, 0.0f, 0.0f), pbr_mat.metalness);

    float3 diffuse_color = KD * pbr_mat.base_color * irradiance_color;
    float3 specular_color = prefiltered_color * (BRDFLUT.x * F0 + BRDFLUT.y);

    return diffuse_color + specular_color;
}


float4 PS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 albedo_metalness = gGeometryAlbedoMetalness.Sample(gSamAnisotropicWrap, texcoord);
    float4 world_normal_roughness = gGeometryNormalRoughness.Sample(gSamAnisotropicWrap, texcoord);
    float3 world_position = gGeometryWorldPosition.Sample(gSamAnisotropicWrap, texcoord).rgb;

    PBRMaterial pbr_mat;
    pbr_mat.base_color = albedo_metalness.rgb;
    pbr_mat.opacity = 1.0f;
    pbr_mat.specular_color = float3(0.0f, 0.0f, 0.0f);
    pbr_mat.specular_strength = 0.0f;
    pbr_mat.specular_tint = 0.0f;
    pbr_mat.anisotropic = 0.0f;
    pbr_mat.metalness = albedo_metalness.a;
    pbr_mat.roughness = world_normal_roughness.a;

    float3 view_dir = normalize(gEyeWorldPos.xyz - world_position);
    float3 normal = normalize(world_normal_roughness.rgb);
    float normal_dot_view = max(0.0f, dot(normal, view_dir));

    float4 final_color = float4(pbr_mat.base_color, 1.0f);

    if (gNoPreprocess == 0)
    {
        float2 brdf = gBRDFLUT.Sample(gSamAnisotropicWrap, float2(normal_dot_view, pbr_mat.roughness)).rg;
        float3 prefiltered_color = sample_prefiltered_color(view_dir, normal, pbr_mat.roughness);
        float3 irradiance_color = gIrradianceMap.Sample(gSamAnisotropicWrap, normal).rgb;
        float3 ambient_color = ambient_brdf(pbr_mat, irradiance_color, prefiltered_color, brdf, normal_dot_view);
        final_color.rgb = ambient_color;
    }

    return final_color;
}