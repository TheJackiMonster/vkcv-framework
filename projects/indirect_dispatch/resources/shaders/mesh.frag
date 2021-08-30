#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0)    uniform texture2D   albedoTexture;
layout(set=0, binding=1)    uniform sampler     textureSampler;

void main()	{
    vec3    albedo  = texture(sampler2D(albedoTexture, textureSampler), passUV).rgb;
    vec3    N       = normalize(passNormal);
    float   light   = max(N.y * 0.5 + 0.5, 0);
    outColor        = light * albedo;
}