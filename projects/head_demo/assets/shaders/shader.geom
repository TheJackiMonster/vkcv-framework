#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 geomNormal[];

layout(location = 0) out vec3 passNormal;

layout(set=1, binding=0) uniform clipBuffer {
    float clipLimit;
    float clipX;
    float clipY;
    float clipZ;
};

layout( push_constant ) uniform constants{
    mat4 mvp;
};

#define CLIP_SCALE 10000.0f

vec4 clipPosition(vec4 pos) {
    return vec4(
        min(clipX, pos.x),
        min(clipY, pos.y),
        min(clipZ, pos.z),
        1.0f / CLIP_SCALE
    );
}

vec4 clipByLimit(vec4 pos) {
    if (pos.x / pos.w < clipLimit) {
        return vec4(pos.xyz / pos.w, 1.0f);
    } else {
        return vec4(clipLimit, pos.y / pos.w, pos.z / pos.w, 1.0f);
    }
}

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
        v0 = clipByLimit(mvp * (v0 * CLIP_SCALE));
        v1 = clipByLimit(mvp * (v1 * CLIP_SCALE));
        v2 = clipByLimit(mvp * (v2 * CLIP_SCALE));

        if ((v0.x < clipLimit) || (v1.x < clipLimit) || (v2.x < clipLimit)) {
            gl_Position = v0;
            passNormal = geomNormal[0];
            EmitVertex();

            gl_Position = v1;
            passNormal = geomNormal[1];
            EmitVertex();

            gl_Position = v2;
            passNormal = geomNormal[2];
            EmitVertex();

            EndPrimitive();
        }
    }
}
