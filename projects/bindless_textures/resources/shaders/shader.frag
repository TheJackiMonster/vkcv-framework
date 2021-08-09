#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform texture2D  materialTextures[];
layout(set=0, binding=1) uniform sampler    textureSampler;

void main()	{
	outColor = texture(sampler2D(materialTextures[1], textureSampler), passUV).rgb;
}