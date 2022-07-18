#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(points) in;
layout (triangle_strip, max_vertices = 4) out;

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
layout(location = 3) out float passDensity;
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
    const uint useCount = geomUseCount[0];

    if (useCount <= 1) {
        return;
    }

    vec4 viewPositions [2];

    for (uint i = 0; i < 2; i++) {
        const vec3 position = points[startIndex + i].position;

        viewPositions[i] = view * vec4(position, 1);
    }

    vec2 pos = viewPositions[0].xy;
    vec2 dir = normalize(viewPositions[1].xy - pos);

    for (uint i = 0; i < useCount; i++) {
        const float u = float(i + 1) / float(useCount);

        const vec3 position = points[startIndex + i].position;
        const float size = points[startIndex + i].size;

        vec4 viewPos = view * vec4(position, 1);

        if (i > 0) {
            dir = normalize(viewPos.xy - pos);
            pos = viewPos.xy;
        }

        vec2 offset = vec2(-dir.y, dir.x) * size;
        float density = 1.0f;// (1.0f - u * u);

        passPos = vec3(u, -1.0f, -1.0f);
        passView = viewPos.xyz - vec3(offset, 0);
        passColor = mix(color, trailColor, u);
        passDensity = density;
        passSmokeIndex = int(id);

        gl_Position = projection * viewPos - vec4(offset, 0, 0);
        EmitVertex();

        passPos = vec3(u, +1.0f, -1.0f);
        passView = viewPos.xyz + vec3(offset, 0);
        passColor = mix(color, trailColor, u);
        passDensity = density;
        passSmokeIndex = int(id);

        gl_Position = projection * viewPos + vec4(offset, 0, 0);
        EmitVertex();

        if (i > 0) {
            EndPrimitive();
        } else {
            continue;
        }

        if (i + 1 >= useCount) {
            break;
        }

        passPos = vec3(u, -1.0f, -1.0f);
        passView = viewPos.xyz - vec3(offset, 0);
        passColor = mix(color, trailColor, u);
        passDensity = density;
        passSmokeIndex = int(id);

        gl_Position = projection * viewPos - vec4(offset, 0, 0);
        EmitVertex();

        passPos = vec3(u, +1.0f, -1.0f);
        passView = viewPos.xyz + vec3(offset, 0);
        passColor = mix(color, trailColor, u);
        passDensity = density;
        passSmokeIndex = int(id);

        gl_Position = projection * viewPos + vec4(offset, 0, 0);
        EmitVertex();
    }
}