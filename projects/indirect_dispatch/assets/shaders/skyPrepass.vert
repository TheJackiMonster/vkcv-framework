#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout( push_constant ) uniform constants{
    mat4 viewProjection;
    mat4 viewProjectionPrevious;
};

layout(location = 0) out vec4 passNDC;
layout(location = 1) out vec4 passNDCPrevious;

void main()	{
	gl_Position     = viewProjection * vec4(inPosition, 0.0);
    gl_Position.w   = gl_Position.z;
    
    passNDC         = gl_Position;
    
    passNDCPrevious     = viewProjectionPrevious * vec4(inPosition, 0.0);
    passNDCPrevious.w   = passNDCPrevious.z;
}