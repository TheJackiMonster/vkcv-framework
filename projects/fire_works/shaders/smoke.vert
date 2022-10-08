#version 450
#extension GL_GOOGLE_include_directive : enable

#include "smoke.inc"

layout(set=0, binding=0, std430) readonly buffer smokeBuffer {
    smoke_t smokes [];
};

layout(location = 0) in vec3 vertexPos;

layout(location = 0) out vec3 passPos;
layout(location = 1) out vec3 passDir;
layout(location = 2) out vec3 passColor;
layout(location = 3) out float passDensity;
layout(location = 4) out flat int passSmokeIndex;

layout( push_constant ) uniform constants{
    mat4 mvp;
    vec3 camera;
};

void main()	{
    vec3 position = smokes[gl_InstanceIndex].position;
    float size = smokes[gl_InstanceIndex].size;
    vec3 color = smokes[gl_InstanceIndex].color;

    vec3 pos = position + vertexPos * size;

    passPos = vertexPos;
    passDir = pos - camera;
    passColor = color;
    passDensity = smokeDensity(size);
    passSmokeIndex = gl_InstanceIndex;

    // transform position into projected view space
    gl_Position = mvp * vec4(pos, 1);
}