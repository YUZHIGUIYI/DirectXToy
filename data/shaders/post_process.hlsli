cbuffer CBSettings : register(b0)
{
    int g_BlurRadius;
    
    // Supports up to 31 fuzzy weights
    float4 g_Weights[8];

    // Does not belong to constant buffer
    static float s_Weights[32] = (float[32]) g_Weights;
}

Texture2D g_Input : register(t0);
Texture2D g_EdgeInput : register(t1);

RWTexture2D<float4> g_Output : register(u0);

// Linear filter and wrap 
SamplerState g_SamLinearWrap : register(s0);
// Point filter and clamp
SamplerState g_SamPointClamp : register(s1);

#define MAX_BLUR_RADIUS 15
#define N 256
#define CacheSize (N + 2 * MAX_BLUR_RADIUS)