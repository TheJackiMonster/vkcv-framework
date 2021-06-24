#version 450
#extension GL_ARB_separate_shader_objects : enable

#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 passUV;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
	gl_Position = mvp * vec4(inPosition, 1.0);
    passUV = inUV;
}