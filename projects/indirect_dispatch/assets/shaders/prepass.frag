#version 450
#extension GL_ARB_separate_shader_objects   : enable
#extension GL_GOOGLE_include_directive      : enable

#include "motionVector.inc"

layout(location = 0) in vec4 passNDC;
layout(location = 1) in vec4 passNDCPrevious;

layout(location = 0) out vec2 outMotion;

void main()	{
	outMotion = computeMotionVector(passNDC, passNDCPrevious);
}