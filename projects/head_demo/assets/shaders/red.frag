#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;

layout(location = 0) out vec3 outColor;

void main()	{
    outColor = (vec3(0.3f, 0, 0) + max(dot(passNormal, vec3(1.0f, -1.0f, 0.5f)), 0.0f) * vec3(0.7f, 0, 0));
}