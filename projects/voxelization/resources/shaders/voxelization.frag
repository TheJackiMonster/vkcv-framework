#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "voxel.inc"
#include "perMeshResources.inc"
#include "lightInfo.inc"

layout(location = 0) in vec3 passPos;
layout(location = 1) in vec2 passUV;
layout(location = 2) in vec3 passN;

layout(set=0, binding=0, std430) buffer voxelizationBuffer{
    uint packedVoxelData[];
};

layout(set=0, binding=1) uniform voxelizationInfo{
    VoxelInfo voxelInfo;
};

layout(set=0, binding=2, r8) uniform image3D voxelImage;

layout(set=0, binding=3) uniform sunBuffer {
    LightInfo lightInfo;
};

vec3 worldToVoxelCoordinates(vec3 world, VoxelInfo info){
    return (world - info.offset) / info.extent + 0.5f;
}

ivec3 voxelCoordinatesToUV(vec3 voxelCoordinates, ivec3 voxelImageResolution){
    return ivec3(voxelCoordinates * voxelImageResolution);
}

void main()	{
    vec3 voxelCoordinates = worldToVoxelCoordinates(passPos, voxelInfo);
    ivec3 voxelImageSize = imageSize(voxelImage);
    ivec3 UV = voxelCoordinatesToUV(voxelCoordinates, voxelImageSize);
    if(any(lessThan(UV, ivec3(0))) || any(greaterThanEqual(UV, voxelImageSize))){
        return;
    }
    uint flatIndex = flattenVoxelUVToIndex(UV, voxelImageSize);

    vec3 albedo = texture(sampler2D(albedoTexture, textureSampler), passUV).rgb;
    
    vec3 N      = normalize(passN);
    float NoL   = clamp(dot(N, lightInfo.L), 0, 1);
    vec3 color  = albedo * NoL;
    
    atomicMax(packedVoxelData[flatIndex], packVoxelInfo(color));
}