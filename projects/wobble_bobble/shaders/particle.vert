#version 450
#extension GL_GOOGLE_include_directive : enable

#include "particle.inc"

layout(set=0, binding=0, std430) buffer particleBuffer {
    Particle particles [];
};

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 passPos;

void main()	{
    vec3 position = particles[gl_InstanceIndex].minimal.position;
    float size = particles[gl_InstanceIndex].minimal.size;

    passPos = vertexPos;
    gl_Position = vec4(position + vec3(vertexPos * size, 0), 1);
}