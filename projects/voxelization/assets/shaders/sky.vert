#version 450
#extension GL_ARB_separate_shader_objects : enable

const vec2 positions[3] = {
    vec2(-1, -1),
    vec2(-1, 4),
    vec2(4, -1)
};

void main()	{
	gl_Position = vec4(positions[gl_VertexIndex], 1, 1);
}