#version 450
#extension GL_GOOGLE_include_directive : enable

#include "smoke.inc"

layout(set=0, binding=0, std430) readonly buffer smokeBuffer {
    smoke_t smokes [];
};

layout(set=0, binding=1, std430) buffer smokeIndexBuffer {
    uint smokeIndex;
};

layout(location = 0) in vec3 vertexPos;

layout(location = 0) out vec3 passPos;
layout(location = 1) out flat vec3 passColor;
layout(location = 2) out flat float passSize;
layout(location = 3) out flat int passTextureIndex;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
    vec3 position = smokes[gl_InstanceIndex].position;
    float size = smokes[gl_InstanceIndex].size;
    vec3 color = smokes[gl_InstanceIndex].color;

    passPos = vertexPos;
    passColor = color;
    passSize = size;
    passTextureIndex = gl_InstanceIndex;

    // transform position into projected view space
    gl_Position = mvp * vec4(position + vertexPos * size, 1);
}