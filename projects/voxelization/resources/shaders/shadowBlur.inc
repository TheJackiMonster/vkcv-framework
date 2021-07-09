#ifndef SHADOW_BLUR_INC
#define SHADOW_BLUR_INC

vec4 blurMomentShadowMap1D(ivec2 coord, ivec2 blurDirection, texture2D srcTexture, sampler depthSampler){
    
    int blurRadius  = 9;
    int minOffset   = -(blurRadius-1) / 2;
    int maxOffset   = -minOffset;
    
    vec2 pixelSize = vec2(1) / textureSize(sampler2D(srcTexture, depthSampler), 0);
    
    float wTotal = 0;
    vec4 moments = vec4(0);
    
    float weights1D[4] = { 0.5, 0.25, 0.125, 0.0625 };    // gaussian
    
    for(int i = minOffset; i <= maxOffset; i++){
        vec2 uv = (coord + i * blurDirection) * pixelSize;
        uv      += 0.5 * pixelSize * blurDirection * sign(i); // half pixel shift to take advantage of bilinear filtering
        float w = weights1D[abs(i)];
        moments += w * texture(sampler2D(srcTexture, depthSampler), uv);
        wTotal  += w;
    }
    return moments / wTotal;
}

#endif // #ifndef SHADOW_BLUR_INC