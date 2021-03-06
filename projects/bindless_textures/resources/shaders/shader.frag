#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;
layout(location = 2) in flat int passTextureIndex;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform sampler    textureSampler;
layout(set=0, binding=1) uniform texture2D  materialTextures[];

void main()	{
	outColor =  texture(sampler2D(materialTextures[nonuniformEXT(passTextureIndex)], textureSampler), passUV).rgb;
}