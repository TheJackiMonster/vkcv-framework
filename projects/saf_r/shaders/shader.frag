#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform texture2D  tex;
layout(set=0, binding=1) uniform sampler    textureSampler;

void main() {
	outColor = fragColor;
	outColor = texture(sampler2D(tex, textureSampler), texCoord).rgb;
}