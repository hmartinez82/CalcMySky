#version 330
#extension GL_ARB_shading_language_420pack : require

#include "const.h.glsl"
#include "phase-functions.h.glsl"
#include "common-functions.h.glsl"
#include "texture-coordinates.h.glsl"

uniform sampler2D transmittanceTexture;
uniform float sunAngularRadius;

vec4 transmittanceToAtmosphereBorder(const float cosViewZenithAngle, const float altitude)
{
    const vec2 texCoords=transmittanceTexVarsToTexCoord(cosViewZenithAngle, altitude);
    return texture(transmittanceTexture, texCoords);
}

// Assumes that the endpoint of view ray doesn't intentionally exit atmosphere.
vec4 transmittance(const float cosViewZenithAngle, const float altitude, const float dist,
                   const bool viewRayIntersectsGround)
{
    const float r=earthRadius+altitude;
    // Clamping only guards against rounding errors here, we don't try to handle view ray endpoint
    // in space here.
    const float altAtDist=clampAltitude(sqrt(sqr(dist)+sqr(r)+2*r*dist*cosViewZenithAngle)-earthRadius);
    const float cosViewZenithAngleAtDist=clampCosine((r*cosViewZenithAngle+dist)/(earthRadius+altAtDist));

    // min() clamps transmittance to <=1, which could otherwise happen to be >1 due to rounding errors in coordinates.
    if(viewRayIntersectsGround)
    {
        return min(transmittanceToAtmosphereBorder(-cosViewZenithAngleAtDist, altAtDist)
                                                /
                   transmittanceToAtmosphereBorder(-cosViewZenithAngle, altitude)
                   ,
                   1.);
    }
    else
    {
        return min(transmittanceToAtmosphereBorder(cosViewZenithAngle, altitude)
                                                /
                   transmittanceToAtmosphereBorder(cosViewZenithAngleAtDist, altAtDist)
                   ,
                   1.);
    }
}

vec4 transmittanceToSun(const float cosSunZenithAngle, float altitude)
{
    if(altitude<0) altitude=0;
    const float sinHorizonZenithAngle = earthRadius/(earthRadius+altitude);
    const float cosHorizonZenithAngle = -sqrt(1-sqr(sinHorizonZenithAngle));
    /* Approximating visible fraction of solar disk by smoothstep between the position of the Sun
     * touching the horizon by its upper part and the position with lower part touching the horizon.
     * The calculation assumes that solar angular radius is small and thus approximately equals its sine.
     * For details, see Bruneton's explanation before GetTransmittanceToSun() in the updated
     * Precomputed Atmospheric Scattering demo.
     */
    return transmittanceToAtmosphereBorder(cosSunZenithAngle, altitude)
                                    *
           smoothstep(-sinHorizonZenithAngle*sunAngularRadius,
                       sinHorizonZenithAngle*sunAngularRadius,
                       cosSunZenithAngle-cosHorizonZenithAngle);
}

vec4 scattering(const sampler3D singleRayleighScatteringTexture, const sampler3D singleMieScatteringTexture,
                const sampler3D multipleScatteringTexture,
                const float cosSunZenithAngle, const float cosViewZenithAngle,
                const float dotViewSun, const float altitude, const bool viewRayIntersectsGround,
                const int scatteringOrder)
{
    if(scatteringOrder==1)
    {
        const vec4 rayleigh = sample4DTexture(singleRayleighScatteringTexture, cosSunZenithAngle, cosViewZenithAngle,
                                              dotViewSun, altitude, viewRayIntersectsGround);
        const vec4 mie      = sample4DTexture(singleMieScatteringTexture, cosSunZenithAngle, cosViewZenithAngle,
                                              dotViewSun, altitude, viewRayIntersectsGround);
        return rayleigh*rayleighPhaseFunction(dotViewSun)+mie*miePhaseFunction(dotViewSun);
    }
    else
    {
        return sample4DTexture(multipleScatteringTexture, cosSunZenithAngle, cosViewZenithAngle,
                               dotViewSun, altitude, viewRayIntersectsGround);
    }
}
