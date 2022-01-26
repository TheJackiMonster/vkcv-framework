#version 450
#extension GL_GOOGLE_include_directive : enable

#include "particle.inc"

layout(set=0, binding=0, std430) buffer particleBuffer {
    Particle particles [];
};

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 passPos;
layout(location = 1) out float passMass;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
    vec3 position = particles[gl_InstanceIndex].minimal.position;
    float size = particles[gl_InstanceIndex].minimal.size;

    float mass = particles[gl_InstanceIndex].minimal.mass;

    passPos = vertexPos;
    passMass = mass;

    // align particle to face camera
    gl_Position = mvp * vec4(position, 1);      // transform position into projected view space
    gl_Position.xy += vertexPos * size * 2.0f;  // move position directly in view space
}