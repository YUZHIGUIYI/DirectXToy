#include "post_process.hlsli"

// Linear RGB converts to lighting
float3 RGB2Gray(float3 color)
{
    return dot(color, float3(0.212671f, 0.715160f, 0.072169f));
}

[numthreads(16, 16, 1)]
void CS(int3 DTid : SV_DispatchThreadID)
{
    // Collect the current pixel and adjacent eight pixels
    float4 colors[3][3];
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            int2 xy = DTid.xy + int2(-1 + j, -1 + i);
            colors[i][j] = g_Input[xy];
        }
    }

    // For each color channel, 
    // use Sobel operator to estimate the approximate value of Partial derivative about x
    float4 Gx = -1.0f * colors[0][0] - 2.0f * colors[1][0] - 1.0f * colors[2][0] +
        1.0f * colors[0][2] + 2.0f * colors[1][2] + 1.0f * colors[2][2];

    // For each color channel, 
    // use Sobel operator to estimate the approximate value of Partial derivative about y
    float4 Gy = -1.0f * colors[2][0] - 2.0f * colors[2][1] - 1.0f * colors[2][2] +
        1.0f * colors[0][0] + 2.0f * colors[0][1] + 1.0f * colors[0][2];

    
    // The gradient vector is (Gx, Gy)
    // Calculate the gradient size for each color channel
    // To find the maximum rate of change
    float4 mag = sqrt(Gx * Gx + Gy * Gy);

    // Draw the edges with steep gradients as black, and non edges with flat gradients as white
    mag = 1.0f - float4(saturate(RGB2Gray(mag.xyz)), 0.0f);

    g_Output[DTid.xy] = mag; 
}