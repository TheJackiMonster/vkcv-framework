#version 450

layout(set=0, binding=0, rgba32f) readonly uniform image3D gridImage;

layout(location = 0) in vec2 vertexPos;

layout(location = 0) out vec2 passPos;
layout(location = 1) out float passMass;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

ivec3 actual_mod(ivec3 x, ivec3 y) {
    return x - y * (x/y);
}

void main()	{
    ivec3 gridResolution = imageSize(gridImage);

    ivec3 gridID = ivec3(
        gl_InstanceIndex,
        gl_InstanceIndex / gridResolution.x,
        gl_InstanceIndex / gridResolution.x / gridResolution.y
    );

    gridID = actual_mod(gridID, gridResolution);

    vec3 position = (vec3(gridID) + vec3(0.5f)) / gridResolution;
    float size = 1.0f / length(vec3(gridResolution));

    vec4 gridData = imageLoad(gridImage, gridID);

    float mass = gridData.w;

    passPos = vertexPos;
    passMass = mass;

    // align voxel to face camera
    gl_Position = mvp * vec4(position, 1);      // transform position into projected view space
    gl_Position.xy += vertexPos * (size * 2.0f) * (mass * 100.0f);  // move position directly in view space
}