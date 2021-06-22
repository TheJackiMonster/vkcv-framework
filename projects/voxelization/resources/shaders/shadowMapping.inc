#ifndef SHADOW_MAPPING_INC
#define SHADOW_MAPPING_INC

#include "lightInfo.inc"

float shadowTest(vec3 worldPos, LightInfo lightInfo, texture2D shadowMap, sampler shadowMapSampler){
    vec4 lightPos = lightInfo.lightMatrix * vec4(worldPos, 1);
    lightPos /= lightPos.w;
    lightPos.xy = lightPos.xy * 0.5 + 0.5;
    
    if(any(lessThan(lightPos.xy, vec2(0))) || any(greaterThan(lightPos.xy, vec2(1)))){
        return 1;
    }
    
    lightPos.z = clamp(lightPos.z, 0, 1);
    
    // using exponential shadow mapping
    float shadowMapSample = texture(sampler2D(shadowMap, shadowMapSampler), lightPos.xy).r;
    return clamp(exp(-lightInfo.exponentialWarp * lightPos.z) * shadowMapSample, 0, 1);
}

#endif // #ifndef SHADOW_MAPPING_INC