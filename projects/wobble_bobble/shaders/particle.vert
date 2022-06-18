#version 450
#extension GL_GOOGLE_include_directive : enable

#include "particle.inc"

layout(set=0, binding=0, std430) readonly buffer particleBuffer {
    Particle particles [];
};

layout(location = 0) out vec2 passPos;
layout(location = 1) out float passMass;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
    const vec2 positions[3] = {
        vec2(-1.0f, -1.0f),
        vec2(+0.0f, +1.5f),
        vec2(+1.0f, -1.0f)
    };

    vec3 position = particles[gl_InstanceIndex].minimal.position;
    float size = particles[gl_InstanceIndex].minimal.size;

    float mass = particles[gl_InstanceIndex].minimal.mass;

    passPos = positions[gl_VertexIndex];
    passMass = mass;

    // align particle to face camera
    gl_Position = mvp * vec4(position, 1);      // transform position into projected view space
    gl_Position.xy += positions[gl_VertexIndex] * size * 2.0f;  // move position directly in view space
}