#version 450
#extension GL_ARB_separate_shader_objects   : enable
#extension GL_GOOGLE_include_directive      : enable

#include "common.inc"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 passNormal;
layout(location = 1) out flat uint passTaskIndex;

layout(std430, binding = 0) readonly buffer matrixBuffer {
    ObjectMatrices objectMatrices[];
};

layout( push_constant ) uniform constants {
    uint padding; // pad to same size as mesh shader constants
    uint matrixIndex;
};

void main()	{
	passNormal = inNormal;
    passTaskIndex = 0;

    gl_Position = objectMatrices[matrixIndex].mvp * vec4(inPosition, 1.0);
}