#version 330
#extension GL_ARB_shading_language_420pack : require

#include "const.h.glsl"

// Assumes that if its argument is negative, it's due to rounding errors and
// should instead be zero.
float safeSqrt(const float x)
{
    return sqrt(max(x,0.));
}

// Fixup for possible rounding errors resulting in distance being outside of theoretical bounds
float clampDistance(const float d)
{
    return max(d, 0.);
}

float distanceToAtmosphereBorder(const float cosZenithAngle, const float observerAltitude)
{
    const float Robs=earthRadius+observerAltitude;
    const float Ratm=earthRadius+atmosphereHeight;
    const float discriminant=sqr(Ratm)-sqr(Robs)*(1-sqr(cosZenithAngle));
    return clampDistance(safeSqrt(discriminant)-Robs*cosZenithAngle);
}
