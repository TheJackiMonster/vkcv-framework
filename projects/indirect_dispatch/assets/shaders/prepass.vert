#version 450
#extension GL_ARB_separate_shader_objects : enable

struct vertex_t {
    vec3 position;
    float u;
    vec3 normal;
    float v;
};

layout(std430, set=0, binding=0) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(location = 0) out vec4 passNDC;
layout(location = 1) out vec4 passNDCPrevious;

layout( push_constant ) uniform constants{
    mat4 mvp;
    mat4 mvpPrevious;
};

void main()	{
	gl_Position     = mvp * vec4(vertices[gl_VertexIndex].position, 1.0);
	passNDC         = gl_Position;
    passNDCPrevious = mvpPrevious * vec4(vertices[gl_VertexIndex].position, 1.0);
}