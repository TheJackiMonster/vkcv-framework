#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec4 passNDC;
layout(location = 1) out vec4 passNDCPrevious;

layout( push_constant ) uniform constants{
    mat4 mvp;
    mat4 mvpPrevious;
};

void main()	{
	gl_Position     = mvp * vec4(inPosition, 1.0);
	passNDC         = gl_Position;
    passNDCPrevious = mvpPrevious * vec4(inPosition, 1.0);
}