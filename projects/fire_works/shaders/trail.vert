#version 450
#extension GL_GOOGLE_include_directive : enable

#include "trail.inc"

layout(set=0, binding=0, std430) readonly buffer trailBuffer {
    trail_t trails [];
};

#include "particle.inc"

layout(set=2, binding=0, std430) readonly buffer particleBuffer {
    particle_t particles [];
};

layout(location = 0) out vec3 geomColor;
layout(location = 1) out uint geomTrailIndex;
layout(location = 2) out vec3 geomTrailColor;
layout(location = 3) out uint geomStartIndex;
layout(location = 4) out uint geomUseCount;

void main()	{
    const uint particleIndex = trails[gl_InstanceIndex].particleIndex;
    const float lifetime = trails[gl_InstanceIndex].lifetime;

    geomColor = particles[particleIndex].color;
    geomTrailIndex = gl_InstanceIndex;
    geomTrailColor = trails[gl_InstanceIndex].color;
    geomStartIndex = trails[gl_InstanceIndex].startIndex;

    const uint useCount = trails[gl_InstanceIndex].useCount;

    if (lifetime > 0.0f) {
        geomUseCount = useCount;
    } else {
        geomUseCount = 0;
    }

    gl_Position = vec4(gl_InstanceIndex, lifetime, useCount, 0.0f);
}