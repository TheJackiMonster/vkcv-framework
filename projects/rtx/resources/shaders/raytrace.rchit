#version 460
#extension GL_EXT_ray_tracing : require


layout(binding = 3, set = 0) buffer rtxVertices
{
    vec3 vertices[];
};

layout(binding = 4, set = 0) buffer rtxIndices
{
    uint indices[];
};

void main() {
    int b = 42;
}
