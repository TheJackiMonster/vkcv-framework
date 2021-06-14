#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 particle;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()
{
	gl_Position = mvp * vec4(particle, 1.0);
}