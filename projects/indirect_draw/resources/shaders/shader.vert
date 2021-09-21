#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(std430, binding=1) readonly buffer uModel {
    mat4 modelMatrix[];
};

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;
layout(location = 2) out flat uint passDrawIndex;

layout( push_constant ) uniform constants{
    mat4 vp;
};

void main()
{
	gl_Position = vp * modelMatrix[gl_DrawID] * vec4(inPosition, 1.0);
	passNormal  = inNormal;
    passUV      = inUV;
    passDrawIndex = gl_DrawID;
}