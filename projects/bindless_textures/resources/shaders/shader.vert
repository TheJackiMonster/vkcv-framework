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

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;
layout(location = 2) out flat int passTextureIndex;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()
{
	gl_Position = mvp * vec4(vertices[gl_VertexIndex].position, 1.0);
	passNormal  = vertices[gl_VertexIndex].normal;
    passUV      = vec2(vertices[gl_VertexIndex].u, vertices[gl_VertexIndex].v);

    passTextureIndex = (gl_VertexIndex / 4);

    /*
    if(inNormal.x > 0.9)
        passTextureIndex = 0;

    if(inNormal.x < -0.9)
        passTextureIndex = 1;

    if(inNormal.y > 0.9)
        passTextureIndex = 2;

    if(inNormal.y < -0.9)
        passTextureIndex = 3;

    if(inNormal.z > 0.9)
        passTextureIndex = 4;

    if(inNormal.z < -0.9)
        passTextureIndex = 5;
    */
}