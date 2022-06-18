#version 450
#extension GL_ARB_separate_shader_objects : enable

struct vertex_t {
	vec3 position;
	float u;
	vec3 normal;
	float v;
};

layout(std430, set=0, binding=0) buffer buffer_vertexBuffer {
	vertex_t vertices [];
};

layout(location = 0) out vec3 geomNormal;

void main()	{
	gl_Position = vec4(vertices[gl_VertexIndex].position, 1.0);
	geomNormal  = vertices[gl_VertexIndex].normal;
}