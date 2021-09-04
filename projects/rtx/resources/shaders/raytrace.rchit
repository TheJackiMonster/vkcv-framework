#version 460
#extension GL_EXT_ray_tracing : require

layout(binding = 3, set 0) buffer vertices
{
    vec3 vertices[];
};

layout(binding = 4, set 0) buffer indices
{
    uint indices[];
};

void main() {

}
