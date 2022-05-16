#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec3 passEdge;

layout(location = 0) out vec3 outColor;

void main()	{
    outColor = (0.1f + max(dot(passNormal, vec3(1.0f, -1.0f, 0.5f)), 0.0f) * 0.9f) * (passEdge + 0.5f);
}