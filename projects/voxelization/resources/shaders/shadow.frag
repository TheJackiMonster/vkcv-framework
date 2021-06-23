#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "shadowMapping.inc"

layout(set=0, binding=0) uniform LightInfoBuffer {
    LightInfo lightInfo;
};

layout(location = 0) out vec4 outMoments;

layout(location = 0) in vec4 passPos;

void main()	{
    float z         = passPos.z / passPos.w;
    vec2  zWarped   = applyDepthWarp(z,     lightInfo.warps);
    outMoments      = vec4(zWarped, zWarped*zWarped);
}