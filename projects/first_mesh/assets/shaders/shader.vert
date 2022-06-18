#version 450
#extension GL_ARB_separate_shader_objects : enable

struct vertex_t {
    vec3 position;
    float u;
    vec3 normal;
    float v;
};

layout(std430, set=0, binding=2) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
	gl_Position = mvp * vec4(vertices[gl_VertexIndex].position, 1.0);
	passNormal  = vertices[gl_VertexIndex].normal;
    passUV      = vec2(vertices[gl_VertexIndex].u, vertices[gl_VertexIndex].v);
}