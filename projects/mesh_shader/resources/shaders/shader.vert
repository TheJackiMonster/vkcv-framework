#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 passNormal;
layout(location = 1) out uint dummyOutput;

layout( push_constant ) uniform constants{
    mat4 mvp;
    uint padding; // pad to same size as mesh shader constants
};

void main()	{
	gl_Position = mvp * vec4(inPosition, 1.0);
	passNormal  = inNormal;
    
    dummyOutput = padding * 0;  // padding must be used, else compiler shrinks constant size
}