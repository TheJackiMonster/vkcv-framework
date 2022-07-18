#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(points) in;
layout (triangle_strip, max_vertices = 32) out;

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
layout(location = 1) out vec3 passView;
layout(location = 2) out vec3 passColor;
layout(location = 3) out float passSize;
layout(location = 4) out flat int passSmokeIndex;

layout( push_constant ) uniform constants{
    mat4 view;
    mat4 projection;
};

void main() {
    const vec3 color = geomColor[0];
    const uint id = geomTrailIndex[0];

    const vec3 trailColor = geomTrailColor[0];

    const uint startIndex = geomStartIndex[0];
    const uint useCount = min(geomUseCount[0], 16);

    if (useCount <= 1) {
        return;
    }

    vec4 viewPositions [16];
    float sizes [16];

    for (uint i = 0; i < useCount; i++) {
        const vec3 position = points[startIndex + i].position;
        const float size = points[startIndex + i].size;

        viewPositions[i] = view * vec4(position, 1);
        sizes[i] = size;
    }

    vec2 pos = viewPositions[0].xy;
    vec2 dir = normalize(viewPositions[1].xy - pos);

    for (uint i = 0; i < useCount; i++) {
        const float u = float(i + 1) / float(16);

        vec4 viewPos = viewPositions[i];
        float size = sizes[i];

        if (i > 0) {
            dir = normalize(viewPos.xy - pos);
            pos = viewPos.xy;
        }

        vec2 offset = vec2(-dir.y, dir.x) * size;

        passPos = vec3(u, -1.0f, -1.0f);
        passView = viewPos.xyz - vec3(offset, 0);
        passColor = mix(color, trailColor, u);
        passSize = size;
        passSmokeIndex = int(id);

        gl_Position = projection * viewPos - vec4(offset, 0, 0);
        EmitVertex();

        passPos = vec3(u, +1.0f, -1.0f);
        passView = viewPos.xyz + vec3(offset, 0);
        passColor = mix(color, trailColor, u);
        passSize = size;
        passSmokeIndex = int(id);

        gl_Position = projection * viewPos + vec4(offset, 0, 0);
        EmitVertex();
    }

    EndPrimitive();
}