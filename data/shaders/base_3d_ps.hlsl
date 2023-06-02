#include "common.hlsli"

// Pixel shader
float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{
    // Perform alpha clip in advance
    float4 texColor = g_DiffuseMap.Sample(g_Sam, pIn.tex);
    clip(texColor.a - 0.1f);

    // Initialize normal vector
    pIn.normalW = normalize(pIn.normalW);

    float3 toEyeW = normalize(g_EyePosW - pIn.posW);

    // Initialize 
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 D = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 S = float4(0.0f, 0.0f, 0.0f, 0.0f);
    int i;

    // Directional lights
    [unroll]
    for (i = 0; i < 2; ++i)
    {
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    // Point lights
    [unroll]
    for (i = 0; i < 2; ++i)
    {
        ComputePointLight(g_Material, g_PointLight[i], pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.diffuse.a;

    return litColor;
}

