#ifndef SHADOW_MAPPING_INC
#define SHADOW_MAPPING_INC

#include "lightInfo.inc"

vec2 applyDepthWarp(float z, vec2 warp){
    z               = 2 * z - 1;
    float positive  =  exp( z * warp.x);
    float negative  = -exp(-z * warp.y);
    return vec2(positive, negative);
}

float rescale(float a, float b, float v)
{
    return clamp((v - a) / (b - a), 0, 1);
}

float reduceVSMBleeding(float shadowValue, float newMin)
{
   return rescale(newMin, 1.0f, shadowValue);
}

float chebyshevInequality(float mean, float meanSquare, float sampleIn){
    float variance  = meanSquare -  mean * mean;
    float d         = sampleIn - mean;
    float pMax      = clamp(variance / (variance + d*d), 0, 1);
    pMax            = reduceVSMBleeding(pMax, 0.1);
    
    return pMax;
}

float shadowTest(vec3 worldPos, LightInfo lightInfo, texture2D shadowMap, sampler shadowMapSampler){
    vec4 lightPos = lightInfo.lightMatrix * vec4(worldPos, 1);
    lightPos /= lightPos.w;
    lightPos.xy = lightPos.xy * 0.5 + 0.5;
    
    if(any(lessThan(lightPos.xy, vec2(0))) || any(greaterThan(lightPos.xy, vec2(1)))){
        return 1;
    }
    
    lightPos.z          = clamp(lightPos.z, 0, 1);
    vec2 warpedSample   = applyDepthWarp(lightPos.z, lightInfo.warps);
    
    // using exponential variance shadow mapping
    vec4 shadowMapSample    = texture(sampler2D(shadowMap, shadowMapSampler), lightPos.xy);
    vec2 positiveMoments    = shadowMapSample.rb;
    vec2 negativeMoments    = shadowMapSample.ga;

    float s1                = chebyshevInequality(positiveMoments.r, positiveMoments.g, warpedSample.x);
    float s2                = chebyshevInequality(negativeMoments.r, negativeMoments.g, warpedSample.y);

    return min(s1, s2);
}

#endif // #ifndef SHADOW_MAPPING_INC