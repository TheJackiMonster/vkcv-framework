#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passPos;

layout(set=0, binding=0, r8) uniform image3D  voxelImage;
layout(set=0, binding=1) uniform voxelizationInfo{
    float extent;
} voxelInfo;

vec3 worldToVoxelCoordinates(vec3 world, float voxelExtent){
    return world / voxelExtent + 0.5f;
}

ivec3 voxelCoordinatesToUV(vec3 voxelCoordinates, ivec3 voxelImageResolution){
    return ivec3(voxelCoordinates * voxelImageResolution);
}

void main()	{
    vec3 voxelCoordinates = worldToVoxelCoordinates(passPos, voxelInfo.extent);
    ivec3 voxeImageSize = imageSize(voxelImage);
    ivec3 UV = voxelCoordinatesToUV(voxelCoordinates, voxeImageSize);
    if(any(lessThan(UV, ivec3(0))) || any(greaterThanEqual(UV, voxeImageSize))){
        //return;
    }
    imageStore(voxelImage, UV, vec4(1));
}