#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;

layout(location = 0) out vec3 outColor;

layout(set=1, binding=0) uniform texture2D  meshTexture;
layout(set=1, binding=1) uniform sampler    textureSampler;

void main()	{
	outColor = texture(sampler2D(meshTexture, textureSampler), passUV).rgb;
    //outColor = passNormal * 0.5 + 0.5;
}