#version 450
#extension GL_ARB_separate_shader_objects : enable

#extension GL_GOOGLE_include_directive : enable

#include "vertex.inc"

layout(std430, set=2, binding=0) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(location = 0) out vec2 passUV;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
	gl_Position = mvp * vec4(vertices[gl_VertexIndex].position, 1.0);
    passUV = vec2(vertices[gl_VertexIndex].u, vertices[gl_VertexIndex].v);
}