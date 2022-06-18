#version 450
#extension GL_ARB_separate_shader_objects : enable

#extension GL_GOOGLE_include_directive : enable

#include "vertex.inc"

layout(std430, set=2, binding=0) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(location = 0) out vec3 passPos;
layout(location = 1) out vec2 passUV;
layout(location = 2) out vec3 passN;

layout( push_constant ) uniform constants{
    mat4 mvp;
    mat4 model;
};

void main()	{
	gl_Position = mvp * vec4(vertices[gl_VertexIndex].position, 1.0);
    passPos     = (model * vec4(vertices[gl_VertexIndex].position, 1)).xyz;
    passUV      = vec2(vertices[gl_VertexIndex].u, vertices[gl_VertexIndex].v);
    passN       = mat3(model) * vertices[gl_VertexIndex].normal;
}