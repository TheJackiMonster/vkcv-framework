#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 VertexColor;
layout(location = 1) in vec3 passNormal;
layout(location = 0) out vec4 outColor;

void main() {
	vec3 vis = normalize(passNormal) * 0.5 +0.5;
	outColor = vec4(vis , 1);
}