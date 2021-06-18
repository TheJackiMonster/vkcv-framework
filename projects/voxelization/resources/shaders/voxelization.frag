#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "voxel.inc"
#include "perMeshResources.inc"

layout(location = 0) in     vec3 passPos;
layout(location = 1) out    vec2 passUV;

layout(set=0, binding=0, std430) buffer voxelizationBuffer{
    uint packedVoxelData[];
};

layout(set=0, binding=1) uniform voxelizationInfo{
    VoxelInfo voxelInfo;
};

layout(set=0, binding=2, r8) uniform image3D voxelImage;

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
    
    // for some reason the automatic mip level here does not work
    // biasing does not work either
    // as a workaround a fixed, high mip level is chosen
    vec3 color = textureLod(sampler2D(albedoTexture, textureSampler), passUV, 10.f).rgb;
    atomicMax(packedVoxelData[flatIndex], packVoxelInfo(color));
}