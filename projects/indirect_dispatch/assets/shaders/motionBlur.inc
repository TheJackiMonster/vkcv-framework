#ifndef MOTION_BLUR
#define MOTION_BLUR

#include "motionBlurConfig.inc"

// see "A Reconstruction Filter for Plausible Motion Blur", section 2.2
vec2 processMotionVector(vec2 motion, float motionScaleFactor, ivec2 imageResolution){
    // every frame a pixel should blur over the distance it moves
    // as we blur in two directions (where it was and where it will be) we must half the motion 
    vec2 motionHalf     = motion * 0.5;
    vec2 motionScaled   = motionHalf * motionScaleFactor; // scale factor contains shutter speed and delta time
    
    // pixels are anisotropic, so the ratio for clamping the velocity is computed in pixels instead of uv coordinates
    vec2    motionPixel     = motionScaled * imageResolution;
    float   velocityPixels  = length(motionPixel);
    
    float   epsilon         = 0.0001;
    
    // this clamps the motion to not exceed the radius given by the motion tile size
    return motionScaled * max(0.5, min(velocityPixels, motionTileSize)) / (velocityPixels + epsilon);
}

const int ditherSize = 4;

// simple binary dither pattern
// could be optimized to avoid modulo and branch
float dither(ivec2 coord){
    
    bool x = coord.x % ditherSize < (ditherSize / 2);
    bool y = coord.y % ditherSize < (ditherSize / 2);
    
    return x ^^ y ? 1 : 0;
}

#endif // #ifndef MOTION_BLUR