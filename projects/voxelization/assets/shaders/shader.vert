#version 450
#extension GL_ARB_separate_shader_objects : enable

#extension GL_GOOGLE_include_directive : enable

#include "vertex.inc"

layout(std430, set=2, binding=0) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;
layout(location = 2) out vec3 passPos;
layout(location = 3) out vec4 passTangent;

layout( push_constant ) uniform constants{
    mat4 mvp;
    mat4 model;
};

void main()	{
    vec3 position = vertices[gl_VertexIndex].position;
    vec4 tangent = vertices[gl_VertexIndex].tangent;

	gl_Position = mvp * vec4(position, 1.0);
	passNormal  = mat3(model) * vertices[gl_VertexIndex].normal;    // assuming no weird stuff like shearing or non-uniform scaling
    passUV      = vec2(vertices[gl_VertexIndex].u, vertices[gl_VertexIndex].v);
    passPos     = (model * vec4(position, 1)).xyz;
    passTangent = vec4(mat3(model) * tangent.xyz, tangent.w);
}