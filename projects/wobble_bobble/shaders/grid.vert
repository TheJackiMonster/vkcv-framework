#version 450
#extension GL_GOOGLE_include_directive : enable

#include "particle.inc"

layout(set=0, binding=0) uniform texture3D gridImage;
layout(set=0, binding=1) uniform sampler gridSampler;

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 passPos;
layout(location = 1) out vec3 passVelocity;
layout(location = 2) out float passMass;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

ivec3 actual_mod(ivec3 x, ivec3 y) {
    return x - y * (x/y);
}

void main()	{
    ivec3 gridResolution = textureSize(sampler3D(gridImage, gridSampler), 0);

    ivec3 gridID = ivec3(
        gl_InstanceIndex,
        gl_InstanceIndex / gridResolution.x,
        gl_InstanceIndex / gridResolution.x / gridResolution.y
    );

    gridID = actual_mod(gridID, gridResolution);

    vec3 position = (vec3(gridID) + vec3(0.5f)) / gridResolution;
    float size = 1.0f / length(vec3(gridResolution));

    vec4 gridData = texture(sampler3D(gridImage, gridSampler), position);

    float mass = gridData.w;
    float density = mass / sphere_volume(size);
    float alpha = clamp(density / 100000000.0f, 0.0f, 1.0f);

    passPos = vertexPos;
    passVelocity = gridData.xyz;
    passMass = mass;

    // align voxel to face camera
    gl_Position = mvp * vec4(position, 1);      // transform position into projected view space
    gl_Position.xy += vertexPos * mix(0.0f, size * 2.0f, alpha);  // move position directly in view space
}