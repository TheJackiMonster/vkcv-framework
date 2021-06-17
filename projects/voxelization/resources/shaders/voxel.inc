struct VoxelInfo{
    vec3 offset;
    float extent;
};

uint flattenVoxelUVToIndex(ivec3 UV, ivec3 voxelImageSize){
    return UV.x + UV.y * voxelImageSize.x + UV.z *  voxelImageSize.x*  voxelImageSize.y;
}

uint packVoxelInfo(vec3 color){
    uint opaqueBit   = 1 << 31;     
    uint redBits     = uint(color.r * 255);
    uint greenBits   = uint(color.g * 255) << 8;
    uint blueBits    = uint(color.b * 255) << 16;
    return opaqueBit | redBits | greenBits | blueBits;
}

vec4 unpackVoxelInfo(uint packed){
    vec4 rgba;
    rgba.r = (packed >> 0  & 0x000000FF) / 255.f;
    rgba.g = (packed >> 8  & 0x000000FF) / 255.f;
    rgba.b = (packed >> 16 & 0x000000FF) / 255.f;
    rgba.a = packed >> 31; 
    return rgba;
}