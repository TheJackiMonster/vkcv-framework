#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;

layout(location = 0) out vec3 outColor;

void main()	{
	outColor = passNormal;
}