#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec3 passPosObject;

layout(location = 0) out vec3 outColor;

void main()	{
    vec3    albedo  = vec3(sin(passPosObject.y * 100) * 0.5 + 0.5);
    vec3    N       = normalize(passNormal);
    float   light   = max(N.y * 0.5 + 0.5, 0);
    outColor        = light * albedo;
}