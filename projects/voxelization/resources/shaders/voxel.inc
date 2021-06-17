struct VoxelInfo{
    vec3 offset;
    float extent;
};

uint flattenVoxelUVToIndex(ivec3 UV, ivec3 voxelImageSize){
    return UV.x + UV.y * voxelImageSize.x + UV.z *  voxelImageSize.x*  voxelImageSize.y;
}