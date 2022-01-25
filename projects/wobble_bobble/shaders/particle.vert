#version 450
#extension GL_GOOGLE_include_directive : enable

#include "particle.inc"

layout(set=0, binding=0, std430) buffer particleBuffer {
    Particle particles [];
};

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 passPos;
layout(location = 1) out float passMass;

void main()	{
    vec3 position = particles[gl_InstanceIndex].minimal.position;
    float size = particles[gl_InstanceIndex].minimal.size;

    float mass = particles[gl_InstanceIndex].minimal.mass;

    passPos = vertexPos;
    passMass = mass;
    gl_Position = vec4(position + vec3(vertexPos * size * 2.0f, 0), 1);
}