#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "particleShading.inc"

layout(location = 0) in vec2 passTriangleCoordinates;
layout(location = 1) in vec3 passVelocity;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform uColor {
	vec4 color;
} Color;

layout(set=0,binding=1) uniform uPosition{
	vec2 position;
} Position;

void main()
{
    float p = length(passVelocity)/100.f;
    outColor = vec3(0.f+p/3.f, 0.05f+p/2.f, 0.4f+p);

    // make the triangle look like a circle
   outColor *= circleFactor(passTriangleCoordinates);

}
