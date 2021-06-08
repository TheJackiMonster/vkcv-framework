#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform uColor {
	vec4 color;
} Color;

layout(set=0,binding=1) uniform uPosition{
	vec3 position;
} Position;

void main()
{
//	outColor = Color.color;
	outColor = vec4(1.0f, 0.0f,0.f,1.f);
}