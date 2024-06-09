#include "screen_triangle_vs.hlsl"
#include "pbr_common.hlsl"
#include "cascaded_shadow.hlsl"

// Constant normal incidence Fresnel factor for all dielectrics
static const float3 s_fresnel = float3(0.04f, 0.04f, 0.04f);

// PI
static const float s_pi = 3.1415926f;

// Epsilon
static const float s_epsilon = 0.00001f;

// Shlick's approximation of the Fresnel factor.
float3 fresnel_schlick(float3 F0, float normal_dot_view, float roughness)
{
    return F0 + (1.0f - F0) * pow(1.0 - normal_dot_view, 5.0f);
}

// GGX/Towbridge-Reitz normal distribution function
// Use Disney's reparametrization of alpha = roughness^2
float ndf_GGX(float cos_Lh, float roughness)
{
    float alpha = roughness * roughness;
    float alpha_sq = alpha * alpha;
    float denom = (cos_Lh * cos_Lh) * (alpha_sq - 1.0f) + 1.0f;
    
    return alpha_sq / (s_pi * denom * denom);
}

// Single term for separable Schlick-GGX below
float ga_schlick_g1(float cos_theta, float k)
{
    return cos_theta / (cos_theta * (1.0f - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method
float ga_schlick_GGX(float cos_Li, float cos_Lo, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r + r) / 8.0f;
    
    return ga_schlick_g1(cos_Li, k) * ga_schlick_g1(cos_Lo, k);
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

// Calculate cascaded shadow
float get_cascaded_shadow(float3 world_position)
{
    int cascade_index = 0;
    int next_cascade_index = 0;
    float blend_amount = 0.0f;
    float4 shadow_view_position = mul(float4(world_position, 1.0f), gShadowView);
    float view_depth = mul(float4(world_position, 1.0f), gView).z;
    float percent_lit = calculate_cascaded_shadow(shadow_view_position, view_depth, 
                        cascade_index, next_cascade_index, blend_amount);
    
    return percent_lit;
}


float4 PS(float4 homog_position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 albedo_metalness = gGeometryAlbedoMetalness.Sample(gSamAnisotropicWrap, texcoord);
    float4 world_normal_roughness = gGeometryNormalRoughness.Sample(gSamAnisotropicWrap, texcoord);
    float3 world_position = gGeometryWorldPosition.Sample(gSamAnisotropicWrap, texcoord).rgb;

    float percent_lit = get_cascaded_shadow(world_position);

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
    float3 world_normal = normalize(world_normal_roughness.rgb);
    float normal_dot_view = max(0.0f, dot(world_normal, view_dir));
    float cos_Lo = normal_dot_view;

    float4 final_color = float4(pbr_mat.base_color, 1.0f);

    // Fresnel reflectance at normal incidence - for metals, use albedo color
    float3 F0 = lerp(s_fresnel, pbr_mat.base_color, pbr_mat.metalness);
    // Direct lighting
    {
        float3 light_radiance = gLightRadiance;
        float3 Li = -gLightDir;

        // Half-vector between Li and Lo
        float3 Lh = normalize(Li + view_dir);
        
        // Calculate angles beetween surface normal and various light vectors
        float cos_Li = max(0.0f, dot(world_normal, Li));
        float cos_Lh = max(0.0f, dot(world_normal, Lh));

        // Calculate Fresnel term for direct lighting
        float3 F = fresnel_schlick(F0, max(0.0f, dot(Lh, view_dir)), pbr_mat.roughness);
        // Calculate normal distribution for specular BRDF
        float D = ndf_GGX(cos_Lh, pbr_mat.roughness);
        // Calculate geometric attenuation for specular BRDF
        float G = ga_schlick_GGX(cos_Li, cos_Lo, pbr_mat.roughness);

        // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium
        // Metals either reflect or absorb energy, so diffuse contribution is always zero
        // To be energy conserving, scale diffuse BRDF contribution based on Fresnel factor & metalness
        float3 KD = lerp(float3(1.0f, 1.0f, 1.0f) - F, float3(0.0f, 0.0f, 0.0f), pbr_mat.metalness);

        // Lambert diffuse BRDF
        float3 diffuse_BRDF = KD * pbr_mat.base_color;
        // Cook-Torrance specular microfacet BRDF
        float3 specular_BRDF = (F * D * G) / max(s_epsilon, 4.0f * cos_Li * cos_Lo);

        // Total contribution for light
        final_color.rgb = (diffuse_BRDF + specular_BRDF) * light_radiance * cos_Li;
    }

    // Ambient lighting - IBL
    if (gNoPreprocess == 0)
    {
        // Split-sum approximation factors for Cook-Torrance specular BRDF
        float2 brdf = gBRDFLUT.Sample(gSamAnisotropicWrap, float2(normal_dot_view, pbr_mat.roughness)).rg;
        // Sample pre-filtered specular reflection environment at correct mipmap level
        float3 prefiltered_color = sample_prefiltered_color(view_dir, world_normal, pbr_mat.roughness);
        // Sample diffuse irradiance at normal direction
        float3 irradiance_color = gIrradianceMap.Sample(gSamAnisotropicWrap, world_normal).rgb;

        // Fresnel term for ambient lighting 
        // Since we use pre-filtered cubemap and irradiance is coming from different directions
        // Use normal-dot-view(cos Lo) instead of angle with light's half-vector(cos Lh) 
        float3 KS = fresnel_schlick(F0, normal_dot_view, pbr_mat.roughness);
        // Diffuse contribution factor(as with direct lighting)
        float3 KD = lerp(float3(1.0f, 1.0f, 1.0f) - KS, float3(0.0f, 0.0f, 0.0f), pbr_mat.metalness);

        // Irradiance map contains radiance assuming Lambertian BRDF, do not need to scale by 1 / PI here
        float3 diffuse_IBL = KD * pbr_mat.base_color * irradiance_color;
        // Specular IBL contribution
        float3 specular_IBL = prefiltered_color * (brdf.x * F0 + brdf.y);
        // Ambient lighting contribution
        float3 ambient_lighting = diffuse_IBL + specular_IBL;

        final_color.rgb += ambient_lighting;
    }

    // TODO: Shadow
    const float3 light_dirs[4] = {
        float3(-1.0f,  1.0f, -1.0f),
        float3( 1.0f,  1.0f, -1.0f),
        float3( 0.0f, -1.0f,  0.0f),
        float3( 1.0f,  1.0f,  1.0f)
    };

    float lighting = saturate(dot(light_dirs[0], world_normal)) * 0.05f +
                     saturate(dot(light_dirs[1], world_normal)) * 0.05f +
                     saturate(dot(light_dirs[2], world_normal)) * 0.05f +
                     saturate(dot(light_dirs[3], world_normal)) * 0.05f;

    float shadow_lighting = lighting * 0.5f;
    lighting += saturate(dot(-gLightDir, world_normal));
    lighting = lerp(shadow_lighting, lighting, percent_lit);

    return lighting * final_color;
}