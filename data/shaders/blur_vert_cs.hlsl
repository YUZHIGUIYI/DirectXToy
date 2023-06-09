#include "post_process.hlsli"

groupshared float4 g_Cache[CacheSize];

[numthreads(1, N, 1)]
void CS(int3 GTid : SV_GroupThreadID, int3 DTid : SV_DispatchThreadID)
{
    // Reduce bandwidth load by filling in the local thread storage area
    // To blur N pixels, based on the blur radius
    // Need to load N + 2 * BlurRadius pixels

    // This thread group is running N threads
    // In order to obtain an additional 2 * BlurRadius pixels, it is neccessary to get 2 * BlurRadius pixels
    // Every thread collect one more pixel of data
    if (GTid.y < g_BlurRadius)
    {
        // Clamp up boundary
        int y = max(DTid.y - g_BlurRadius, 0);
        g_Cache[GTid.y] = g_Input[int2(DTid.x, y)];
    }

    uint texWidth, texHeight;
    g_Input.GetDimensions(texWidth, texHeight);

    if (GTid.y >= N - g_BlurRadius)
    {
        // Clamp down boundary
        int y = min(DTid.y + g_BlurRadius, texHeight - 1);
        g_Cache[GTid.y + 2 * g_BlurRadius] = g_Input[int2(DTid.x, y)];
    }

    g_Cache[GTid.y + g_BlurRadius] = g_Input[min(DTid.xy, float2(texWidth, texHeight) - 1)];

    // Sync
    GroupMemoryBarrierWithGroupSync();

    // Blend
    float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = -g_BlurRadius; i <= g_BlurRadius; ++i)
    {
        int k = GTid.y + g_BlurRadius + i;

        blurColor += s_Weights[i + g_BlurRadius] * g_Cache[k];
    }

    g_Output[DTid.xy] = blurColor;
}