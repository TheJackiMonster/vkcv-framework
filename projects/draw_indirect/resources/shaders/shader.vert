#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()
{
	gl_Position = mvp * vec4(inPosition, 1.0);
	passNormal  = inNormal;
    passUV      = inUV;
}