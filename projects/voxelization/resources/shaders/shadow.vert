#version 450
#extension GL_ARB_separate_shader_objects : enable

#extension GL_GOOGLE_include_directive : enable

#include "lightInfo.inc"

layout(location = 0) in vec3 inPosition;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

layout(location = 0) out vec4 passPos;

void main()	{
	gl_Position = mvp * vec4(inPosition, 1.0);
    passPos = gl_Position;
}