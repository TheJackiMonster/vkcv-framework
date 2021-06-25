#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "particleShading.inc"

layout(location = 0) in vec2 passTriangleCoordinates;
layout(location = 1) in vec3 passVelocity;
layout(location = 2) in float passlifeTime;

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform uColor {
	vec4 color;
} Color;

layout(set=0,binding=1) uniform uPosition{
	vec2 position;
} Position;


void main()
{
	vec2 mouse = vec2(Position.position.x, Position.position.y);
		outColor = float(passlifeTime < 1) * vec4(1,1,0,0) +
				   float(passlifeTime < 2 && passlifeTime > 1) * vec4(1,passlifeTime * 0.5,0,0) +
				   float(passlifeTime >= 2 && passlifeTime < 2.5f) * vec4(passlifeTime * 0.5,passlifeTime * 0.5,0,0) +
				   float(passlifeTime >= 2.5f) * vec4(1,0,0,0);
   
   // make the triangle look like a circle
   outColor *= circleFactor(passTriangleCoordinates);
   // full color is achieved by additively blending many particles
   outColor *= 0.5; 
}