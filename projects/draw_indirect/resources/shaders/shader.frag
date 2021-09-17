#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;
layout(location = 2) in flat uint passDrawIndex;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform sampler standardSampler;
layout(set=0, binding=2) uniform texture2D baseColorTex[];
// the sponza scene used for this example only had a base color texture
//layout(set=0, binding=2) uniform texture2D metalRoughTex[];
//layout(set=0, binding=3) uniform texture2D normalTex[];
//layout(set=0, binding=4) uniform texture2D occlusionTex[];


void main()
{
    outColor = texture(sampler2D(baseColorTex[passDrawIndex], standardSampler), passUV).rgb;
}