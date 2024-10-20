#ifndef _INTERSECTION_
#define _INTERSECTION_

#define PI 3.14159265f

bool has_intersection_with_circle(float2 o, float2 d, float R)
{
    float A = dot(d, d);
    float B = 2.0f * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4.0f * A * C;
    return (delta >= 0.0f) && ((C <= 0.0f) | (B <= 0.0f));
}

bool has_intersection_with_sphere(float3 o, float3 d, float R)
{
    float A = dot(d, d);
    float B = 2.0f * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4.0f * A * C;
    return (delta >= 0.0f) && ((C <= 0.0f) | (B <= 0.0f));
}

bool find_closest_intersection_with_circle(float2 o, float2 d, float R, out float t)
{
    float A = dot(d, d);
    float B = 2.0f * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4.0f * A * C;
    if (delta < 0.0f)
    {
        return false;
    }
    float sqrt_delta = (C <= 0.0f) ? sqrt(delta) : -sqrt(delta);
    t = (-B + sqrt_delta) / (2.0f * A);
    return (C <= 0.0f) | (B <= 0.0f);
}

bool find_closest_intersection_with_sphere(float3 o, float3 d, float R, out float t)
{
    float A = dot(d, d);
    float B = 2.0f * dot(o, d);
    float C = dot(o, o) - R * R;
    float delta = B * B - 4.0f * A * C;
    if (delta < 0.0f)
    {
        return false;
    }
    float sqrt_delta = (C <= 0.0f) ? sqrt(delta) : -sqrt(delta);
    t = (-B + sqrt_delta) / (2.0f * A);
    return (C <= 0.0f) | (B <= 0.0f);
}

#endif