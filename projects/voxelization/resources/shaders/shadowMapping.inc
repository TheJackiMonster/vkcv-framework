float shadowTest(vec3 worldPos, LightInfo lightInfo, texture2D shadowMap, sampler shadowMapSampler){
    vec4 lightPos = lightInfo.lightMatrix * vec4(worldPos, 1);
    lightPos /= lightPos.w;
    lightPos.xy = lightPos.xy * 0.5 + 0.5;
    
    if(any(lessThan(lightPos.xy, vec2(0))) || any(greaterThan(lightPos.xy, vec2(1)))){
        return 1;
    }
    
    lightPos.z = clamp(lightPos.z, 0, 1);
    
    float shadowMapSample = texture(sampler2D(shadowMap, shadowMapSampler), lightPos.xy).r;
    float bias = 0.01f;
    shadowMapSample += bias;
    return shadowMapSample < lightPos.z ? 0 : 1;
}