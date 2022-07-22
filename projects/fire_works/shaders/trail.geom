#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#define INSTANCE_LEN (16)

layout(points) in;
layout (triangle_strip, max_vertices = (INSTANCE_LEN * 2)) out;
layout(invocations = 8) in;

#include "physics.inc"
#include "point.inc"

layout(set=0, binding=1, std430) readonly buffer pointBuffer {
    point_t points [];
};

layout(location = 0) in vec3 geomColor [1];
layout(location = 1) in uint geomTrailIndex [1];
layout(location = 2) in vec3 geomTrailColor [1];
layout(location = 3) in uint geomStartIndex [1];
layout(location = 4) in uint geomUseCount [1];

layout(location = 0) out vec3 passPos;
layout(location = 1) out vec3 passDir;
layout(location = 2) out vec3 passColor;
layout(location = 3) out float passDensity;
layout(location = 4) out flat int passSmokeIndex;

layout( push_constant ) uniform constants{
    mat4 mvp;
    vec3 camera;
};

void main() {
    const vec3 color = geomColor[0];
    const uint id = geomTrailIndex[0];

    const vec3 trailColor = geomTrailColor[0];

    const uint startIndex = geomStartIndex[0];
    const uint useCount = geomUseCount[0];

    const uint indexOffset = (gl_InvocationID * (INSTANCE_LEN - 1));
    const uint instanceIndex = startIndex + indexOffset;

    uint count = min(INSTANCE_LEN, useCount);

    if ((indexOffset >= useCount) && (indexOffset + INSTANCE_LEN > useCount)) {
        count = indexOffset - useCount;
    }

    if (count <= 1) {
        return;
    }

    vec3 positions [2];
    uint viewIndex = instanceIndex;

    if (viewIndex > startIndex) {
        viewIndex--;
    }

    for (uint i = 0; i < 2; i++) {
        const uint index = (viewIndex + i) % points.length();

        positions[i] = points[index].position;
    }

    vec3 pos = positions[0];
    vec3 dir = normalize(cross(positions[1] - pos, pos - camera));

    const float trailFactor = mediumDensity / friction;

    for (uint i = 0; i < count; i++) {
        const float u = float(indexOffset + i + 1) / float(useCount);

        const uint index = (instanceIndex + i) % points.length();

        const vec3 position = points[index].position;
        const float size = points[index].size;

        if (i > 0) {
            dir = normalize(cross(position - pos, pos - camera));
            pos = position;
        }

        vec3 offset = dir * size;
        float density = trailFactor * (1.0f - u * u) / size;

        const vec3 p0 = position - offset;
        const vec3 p1 = position + offset;

        passPos = vec3(u, -1.0f, -1.0f);
        passDir = vec3(0, 0, 1);
        passColor = mix(color, trailColor, u);
        passDensity = density;
        passSmokeIndex = int(id);

        gl_Position = mvp * vec4(p0, 1);
        EmitVertex();

        passPos = vec3(u, +1.0f, -1.0f);
        passDir = vec3(0, 0, 1);
        passColor = mix(color, trailColor, u);
        passDensity = density;
        passSmokeIndex = int(id);

        gl_Position = mvp * vec4(p1, 1);
        EmitVertex();
    }

    EndPrimitive();
}