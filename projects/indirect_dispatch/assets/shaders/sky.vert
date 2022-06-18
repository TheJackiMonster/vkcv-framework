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

layout( push_constant ) uniform constants{
    mat4 viewProjection;
};

void main()	{
	gl_Position     = viewProjection * vec4(vertices[gl_VertexIndex].position, 0.0);
    gl_Position.w   = gl_Position.z;
}