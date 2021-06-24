#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "perMeshResources.inc"

layout(location = 0) in vec2 passUV;

void main()	{
    float alpha = texture(sampler2D(albedoTexture, textureSampler), passUV).a;
    if(alpha < 0.5){
        discard;
    }
}