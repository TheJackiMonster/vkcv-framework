#version 450
#extension GL_ARB_separate_shader_objects   : enable
#extension GL_GOOGLE_include_directive      : enable

#include "common.inc"

struct vertex_t {
    vec3 position;
    float pad0;
    vec3 normal;
    float pad1;
};

layout(std430, set=0, binding=1) readonly buffer buffer_vertexBuffer {
    vertex_t vertices [];
};

layout(location = 0) out vec3 passNormal;
layout(location = 1) out uint dummyOutput;

layout(std430, binding = 0) readonly buffer matrixBuffer
{
    ObjectMatrices objectMatrices[];
};

layout( push_constant ) uniform constants{
    uint matrixIndex;
    uint padding; // pad to same size as mesh shader constants
};


void main()	{
	gl_Position = objectMatrices[matrixIndex].mvp * vec4(vertices[gl_VertexIndex].position, 1.0);
	passNormal  = vertices[gl_VertexIndex].normal;
    
    dummyOutput = padding * 0;  // padding must be used, else compiler shrinks constant size
}