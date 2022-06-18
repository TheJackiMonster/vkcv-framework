#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 geomNormal[];

layout(location = 0) out vec3 passNormal;

layout(set=2, binding=0) uniform clipBuffer {
    float clipLimit;
    float clipX;
    float clipY;
    float clipZ;
};

layout( push_constant ) uniform constants{
    mat4 mvp;
};

#include "clip.inc"

void main()	{
    vec4 v0 = gl_in[0].gl_Position;
    vec4 v1 = gl_in[1].gl_Position;
    vec4 v2 = gl_in[2].gl_Position;

    v0 = clipPosition(v0 / CLIP_SCALE);
    v1 = clipPosition(v1 / CLIP_SCALE);
    v2 = clipPosition(v2 / CLIP_SCALE);

    float dx = abs(v0.x - clipX) + abs(v1.x - clipX) + abs(v2.x - clipX);
    float dy = abs(v0.y - clipY) + abs(v1.y - clipY) + abs(v2.y - clipY);
    float dz = abs(v0.z - clipZ) + abs(v1.z - clipZ) + abs(v2.z - clipZ);

    if (dx * dy * dz > 0.0f) {
        gl_Position = mvp * (v0 * CLIP_SCALE);
        passNormal = geomNormal[0];
        EmitVertex();

        gl_Position = mvp * (v1 * CLIP_SCALE);
        passNormal = geomNormal[1];
        EmitVertex();

        gl_Position = mvp * (v2 * CLIP_SCALE);
        passNormal = geomNormal[2];
        EmitVertex();

        EndPrimitive();
    }
}
