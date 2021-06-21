#include "brdf.inc"

struct VoxelInfo{
    vec3 offset;
    float extent;
};

struct PackedVoxelData{
    uint color;
    uint normal;
    uint albedo;
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

vec3 voxelCoordinatesToWorldPosition(ivec3 coord, int voxelResolution, VoxelInfo voxelInfo, float voxelHalfSize){
    return (vec3(coord) / voxelResolution - 0.5) * voxelInfo.extent + voxelHalfSize + voxelInfo.offset;
}

// packed voxel data: 
// 1 bit opacity
// 7 bit exposure
// 8 bit blue
// 8 bit green
// 8 bit red
float maxExposure = 16.f;

uint packVoxelColor(vec3 color){
    
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

vec4 unpackVoxelColor(uint packed){
    vec4 rgba;
    rgba.r = (packed >> 0  & 0x000000FF) / 255.f;
    rgba.g = (packed >> 8  & 0x000000FF) / 255.f;
    rgba.b = (packed >> 16 & 0x000000FF) / 255.f;
    rgba.a =  packed  >> 31; 
    
    rgba.rgb *= (packed >> 24 & 0x0000007F) / 127.f * maxExposure; 
    
    return rgba;
}

uint packSNormInto9Bits(float x){
    uint lengthBits = 0x000000FF & uint(abs(x) * 255.f);
    uint signBits   = (x < 0 ? 1 : 0)  << 8;
    return lengthBits | signBits;
}

float unpack9LowBitsIntoSNorm(uint bits){
    bits = (0x000001FF & bits);
    float length    = bits / 255.f;
    float sign      = (bits >> 8) == 0 ? 1 : -1;
    return sign * length;
}

// normals are packed with 9 bits each, 8 for length and 1 for sign
uint packVoxelNormal(vec3 N){
    N           = clamp(N, vec3(0), vec3(1));
    uint xBits  = packSNormInto9Bits(N.x) << 0;
    uint yBits  = packSNormInto9Bits(N.y) << 9;
    uint zBits  = packSNormInto9Bits(N.z) << 18;
    return zBits | yBits | xBits;
}

vec3 unpackVoxelNormal(uint packed){
    vec3 N;
    N.x  = unpack9LowBitsIntoSNorm(packed >> 0);
    N.y  = unpack9LowBitsIntoSNorm(packed >> 9);
    N.z  = unpack9LowBitsIntoSNorm(packed >> 18);
    return normalize(N);
}

uint packUNormInto8Bits(float x){
    return 0x000000FF & uint(abs(x) * 255.f);
}

float unpack8LowBitsIntoUNorm(uint bits){
    bits = (0x000000FF & bits);
    return bits / 255.f;
}

// albedo is packed with 8 bits each
uint packVoxelAlbedo(vec3 albedo){
    albedo      = clamp(albedo, vec3(0), vec3(1));
    uint rBits  = packUNormInto8Bits(albedo.r) << 0;
    uint gBits  = packUNormInto8Bits(albedo.g) << 8;
    uint bBits  = packUNormInto8Bits(albedo.b) << 16;
    return bBits | gBits | rBits;
}

vec3 unpackVoxelAlbedo(uint packed){
    vec3 albedo;
    albedo.r  = unpack8LowBitsIntoUNorm(packed >> 0);
    albedo.g  = unpack8LowBitsIntoUNorm(packed >> 8);
    albedo.b  = unpack8LowBitsIntoUNorm(packed >> 16);
    return albedo;
}

vec3 voxelConeTrace(vec3 direction, vec3 startPosition, float coneAngleRadian, texture3D voxelTexture, sampler voxelSampler, VoxelInfo voxelInfo){

    int voxelResolution =  textureSize(sampler3D(voxelTexture, voxelSampler), 0).x;
    float voxelSize     = voxelInfo.extent / voxelResolution;
    float maxMip        = float(log2(voxelResolution));
    float maxStableMip  = 4;    // must be the same as in Voxelization::voxelizeMeshes
    maxMip              = min(maxMip, maxStableMip);
    float d             = 2 * sqrt(3 * pow(voxelSize, 2));
    vec3 color          = vec3(0);
    float a             = 0;
    
    float coneAngleHalf = coneAngleRadian * 0.5f;
    
    int maxSamples = 128;
    for(int i = 0; i < maxSamples; i++){
        
        vec3 samplePos      = startPosition + d * direction;
        vec3 sampleUV       = worldToVoxelCoordinates(samplePos, voxelInfo);
        
        if(a >= 0.95 || any(lessThan(sampleUV, vec3(0))) || any(greaterThan(sampleUV, vec3(1)))){
            break;
        }
        
        float coneDiameter  = 2 * tan(coneAngleHalf) * d;
        float mip           = log2(coneDiameter / voxelSize);
        mip                 = min(mip, maxMip);
    
        vec4 voxelSample    = textureLod(sampler3D(voxelTexture, voxelSampler), sampleUV , mip);
        
        color               += (1 - a) * voxelSample.rgb;
        a                   += (1 - a) * voxelSample.a;
        
        float minStepSize   = 0.15;
        d                   += max(coneDiameter, minStepSize);
        samplePos           = startPosition + d * direction;
        sampleUV            = worldToVoxelCoordinates(samplePos, voxelInfo);
    }
    return color;
}

float degreeToRadian(float d){
    return d / 180.f * pi;
}

vec3 diffuseVoxelTraceHemisphere(mat3 toSurface, vec3 position, texture3D voxelTexture, sampler voxelSampler, VoxelInfo voxelInfo){
    float coneAngle = degreeToRadian(60.f);
    vec3 diffuseTrace = vec3(0);
    {
        vec3 sampleDirection    = toSurface * vec3(0, 0, 1);
        float weight            = pi / 4.f;
        diffuseTrace            += weight * voxelConeTrace(sampleDirection, position, coneAngle, voxelTexture, voxelSampler, voxelInfo);
    }
    for(int i = 0; i < 6;i++){
        float theta             = 2 * pi / i;
        float phi               = pi / 3;   // 60 degrees
        vec3 sampleDirection    = toSurface * vec3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
        float weight            = pi * (3.f / 4.f) / 6;
        vec3 trace              = voxelConeTrace(sampleDirection, position, coneAngle, voxelTexture, voxelSampler, voxelInfo);
        diffuseTrace            += weight * trace;
    }
    return diffuseTrace;
}