#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform vec4 uColor;

void main()
{
	outColor = uColor;
}