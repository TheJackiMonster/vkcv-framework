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
    outColor = vec3(1.0,1.0,1.0);

    // make the triangle look like a circle
   outColor *= circleFactor(passTriangleCoordinates);

}
