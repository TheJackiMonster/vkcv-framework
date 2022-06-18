#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;
layout(location = 2) in flat uint passDrawIndex;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=2) uniform sampler standardSampler;
layout(set=0, binding=3) uniform texture2D baseColorTex[];


void main()
{
    outColor = texture(sampler2D(baseColorTex[nonuniformEXT(passDrawIndex)], standardSampler), passUV).rgb;
}