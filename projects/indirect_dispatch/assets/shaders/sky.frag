#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 outColor;

void main()	{
	outColor = vec3(0, 0.2, 0.9);
}