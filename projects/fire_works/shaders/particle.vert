#version 450
#extension GL_GOOGLE_include_directive : enable

#include "particle.inc"

layout(set=0, binding=0, std430) readonly buffer particleBuffer {
    particle_t particles [];
};

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 passPos;
layout(location = 1) out flat vec3 passColor;
layout(location = 2) out flat float passLifetime;

layout( push_constant ) uniform constants{
    mat4 mvp;
    uint width;
    uint height;
};

void main()	{
    vec3 position = particles[gl_InstanceIndex].position;
    float lifetime = particles[gl_InstanceIndex].lifetime;
    float size = particles[gl_InstanceIndex].size;
    vec3 color = particles[gl_InstanceIndex].color;

    if (width > height) {
        passPos = vertexPos * vec2(1.0f * width / height, 1.0f);
    } else {
        passPos = vertexPos * vec2(1.0f, 1.0f * height / width);
    }

    passColor = color;
    passLifetime = lifetime;

    // align particle to face camera
    gl_Position = mvp * vec4(position, 1);      // transform position into projected view space
    gl_Position.xy += vertexPos * size * 2.0f;  // move position directly in view space
}