#version 330
#extension GL_ARB_shading_language_420pack : require

#include "const.h.glsl"
#include "multiple-scattering.h.glsl"
#include "texture-coordinates.h.glsl"
#include "common-functions.h.glsl"

uniform int layer;
layout(location=0) out vec4 scatteringDensity;

void main()
{
    const ScatteringTexVars vars=scatteringTexIndicesToTexVars(vec3(gl_FragCoord.xy-vec2(0.5),layer));
    scatteringDensity=computeScatteringDensity(vars.cosSunZenithAngle,vars.cosViewZenithAngle,vars.dotViewSun,
                                               vars.altitude,SCATTERING_ORDER,RADIATION_IS_FROM_GROUND_ONLY);
    if(debugDataPresent()) scatteringDensity=vec4(debugData(),0);
}
