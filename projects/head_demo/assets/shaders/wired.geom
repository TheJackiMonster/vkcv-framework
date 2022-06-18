#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(triangles) in;
layout(points, max_vertices = 1) out;

layout(location = 0) in vec3 geomNormal[];

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec3 passEdge;

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

    if (dx * dy * dz <= 0.0f) {
        v0 = clipByLimit(mvp * gl_in[0].gl_Position);
        v1 = clipByLimit(mvp * gl_in[1].gl_Position);
        v2 = clipByLimit(mvp * gl_in[2].gl_Position);

        if ((v0.x < clipLimit) || (v1.x < clipLimit) || (v2.x < clipLimit)) {
            gl_Position = (v0 + v1 + v2) / 3;
            passNormal = (geomNormal[0] + geomNormal[1] + geomNormal[2]) / 3;
            passEdge = vec3(dx, dy, dz);
            EmitVertex();

            EndPrimitive();
        }
    }
}
