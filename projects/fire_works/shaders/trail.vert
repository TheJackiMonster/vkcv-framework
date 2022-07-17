#version 450
#extension GL_GOOGLE_include_directive : enable

#include "trail.inc"

layout(set=0, binding=0, std430) readonly buffer trailBuffer {
    trail_t trails [];
};

layout(location = 0) out vec3 geomColor;
layout(location = 1) out uint geomTrailIndex;
layout(location = 2) out vec3 geomTrailColor;
layout(location = 3) out uint geomStartIndex;
layout(location = 4) out uint geomUseCount;

void main()	{
    geomColor = vec3(1, 0, 0);
    geomTrailIndex = gl_InstanceIndex;
    geomTrailColor = trails[gl_InstanceIndex].color;
    geomStartIndex = trails[gl_InstanceIndex].startIndex;
    geomUseCount = trails[gl_InstanceIndex].useCount;

    gl_Position = vec4(0.0f);
}