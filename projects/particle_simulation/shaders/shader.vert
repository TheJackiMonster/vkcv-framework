#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout( push_constant ) uniform constants{
    mat4 mvp;
};

vec3 positions[3] = {
vec3(-0.5, 0.5, -1),
vec3( 0.5, 0.5, -1),
vec3(0, -0.5, -1)
};

void main()
{
	gl_Position = mvp * vec4(positions[gl_VertexIndex], 1.0);
}