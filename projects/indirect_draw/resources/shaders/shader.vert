#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_draw_parameters : enable

struct vertex_t {
    vec3 position;
    float u;
    vec3 normal;
    float v;
};

layout(std430, set=0, binding=0) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(std430, set=0, binding=1) readonly buffer uModel {
    mat4 modelMatrix[];
};

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;
layout(location = 2) out flat uint passDrawIndex;

layout( push_constant ) uniform constants{
    mat4 vp;
};

void main()
{
	gl_Position = vp * modelMatrix[gl_DrawID] * vec4(vertices[gl_VertexIndex].position, 1.0);
	passNormal  = vertices[gl_VertexIndex].normal;
    passUV      = vec2(vertices[gl_VertexIndex].u, vertices[gl_VertexIndex].v);
    passDrawIndex = gl_DrawID;
}