#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()
{
	gl_Position = mvp * vec4(position, 1.0);
}