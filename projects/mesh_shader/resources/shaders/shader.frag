#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

struct VertColor
{
	vec3 color;
};

layout(location = 0) in VertColor VertexInput;

void main() {
	outColor = vec4(VertexInput.color, 1);
}