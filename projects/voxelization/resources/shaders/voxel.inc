struct VoxelInfo{
    vec3 offset;
    float extent;
};

uint flattenVoxelUVToIndex(ivec3 UV, ivec3 voxelImageSize){
    return UV.x + UV.y * voxelImageSize.x + UV.z *  voxelImageSize.x*  voxelImageSize.y;
}

vec3 worldToVoxelCoordinates(vec3 world, VoxelInfo info){
    return (world - info.offset) / info.extent + 0.5f;
}

ivec3 voxelCoordinatesToUV(vec3 voxelCoordinates, ivec3 voxelImageResolution){
    return ivec3(voxelCoordinates * voxelImageResolution);
}

// packed voxel data: 
// 1 bit opacity
// 7 bit exposure
// 8 bit blue
// 8 bit green
// 8 bit red
float maxExposure = 16.f;

uint packVoxelInfo(vec3 color){
    
    color               = clamp(color, vec3(0), vec3(maxExposure));
    float maxComponent  = max(max(max(color.r, color.g), color.b), 1.f);
    color               /= maxComponent;
    
    uint opaqueBit      = 1 << 31;
    uint exposureBits   = (0x0000007F & uint(maxComponent / maxExposure * 127)) << 24;
    uint redBits        = (0x000000FF & uint(color.r * 255)) << 0;
    uint greenBits      = (0x000000FF & uint(color.g * 255)) << 8;
    uint blueBits       = (0x000000FF & uint(color.b * 255)) << 16;
    return opaqueBit | exposureBits | blueBits | greenBits | redBits;
}

vec4 unpackVoxelInfo(uint packed){
    vec4 rgba;
    rgba.r = (packed >> 0  & 0x000000FF) / 255.f;
    rgba.g = (packed >> 8  & 0x000000FF) / 255.f;
    rgba.b = (packed >> 16 & 0x000000FF) / 255.f;
    rgba.a =  packed  >> 31; 
    
    rgba.rgb *= (packed >> 24 & 0x0000007F) / 127.f * maxExposure; 
    
    return rgba;
}