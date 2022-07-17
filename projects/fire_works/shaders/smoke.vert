#version 450
#extension GL_GOOGLE_include_directive : enable

#include "smoke.inc"

layout(set=0, binding=0, std430) readonly buffer smokeBuffer {
    smoke_t smokes [];
};

layout(location = 0) in vec3 vertexPos;

layout(location = 0) out vec3 passPos;
layout(location = 1) out vec3 passView;
layout(location = 2) out vec3 passColor;
layout(location = 3) out float passSize;
layout(location = 4) out flat int passSmokeIndex;

layout( push_constant ) uniform constants{
    mat4 view;
    mat4 projection;
};

void main()	{
    vec3 position = smokes[gl_InstanceIndex].position;
    float size = smokes[gl_InstanceIndex].size;
    vec3 color = smokes[gl_InstanceIndex].color;

    vec4 viewPos = view * vec4(position + vertexPos * size, 1);

    passPos = vertexPos;
    passView = viewPos.xyz;
    passColor = color;
    passSize = size;
    passSmokeIndex = gl_InstanceIndex;

    // transform position into projected view space
    gl_Position = projection * viewPos;
}