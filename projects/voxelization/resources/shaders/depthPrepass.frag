#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "perMeshResources.inc"

layout(location = 0) in vec2 passUV;

layout(location = 0) out vec4 outColor; // only used for alpha to coverage, not actually written to

// coverage to alpha techniques explained in: https://bgolus.medium.com/anti-aliased-alpha-test-the-esoteric-alpha-to-coverage-8b177335ae4f
void main()	{
    float alpha         = texture(sampler2D(albedoTexture, textureSampler), passUV).a;
    float alphaCutoff   = 0.5;
    
    // scale alpha to one pixel width
    alpha               = (alpha - alphaCutoff) / max(fwidth(alpha), 0.0001) + 0.5;
    
    outColor.a          = alpha;
}