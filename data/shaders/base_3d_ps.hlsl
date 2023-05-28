#include "common.hlsli"

// Pixel shader
float4 PS(VertexPosHWNormalTex pIn) : SV_Target
{
    // Perform alpha clip in advance
    float4 texColor = g_Texture.Sample(g_SamLinear, pIn.tex);
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
    DirectionalLight dirLight;
    [unroll]
    for (i = 0; i < 2; ++i)
    {
        dirLight = g_DirLight[i];
        [flatten]
        if (g_IsReflection)
        {
            dirLight.direction = mul(dirLight.direction, (float3x3) (g_Reflection));
        }
        ComputeDirectionalLight(g_Material, g_DirLight[i], pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    // Point lights
    PointLight pointLight;
    [unroll]
    for (i = 0; i < 2; ++i)
    {
        pointLight = g_PointLight[i];
        [flatten]
        if (g_IsReflection)
        {
            pointLight.position = (float3) mul(float4(pointLight.position, 1.0f), g_Reflection);
        }
        ComputePointLight(g_Material, pointLight, pIn.posW, pIn.normalW, toEyeW, A, D, S);
        ambient += A;
        diffuse += D;
        spec += S;
    }

    float4 litColor = texColor * (ambient + diffuse) + spec;
    litColor.a = texColor.a * g_Material.diffuse.a;

    return litColor;
}

