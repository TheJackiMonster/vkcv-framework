#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "perMeshResources.inc"
#include "lightInfo.inc"
#include "shadowMapping.inc"

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;
layout(location = 2) in vec3 passPos;
layout(location = 3) in vec4 passTangent;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform sunBuffer {
    LightInfo lightInfo;
};
layout(set=0, binding=1) uniform texture2D  shadowMap;
layout(set=0, binding=2) uniform sampler    shadowMapSampler;

void main()	{

    vec3 albedoTexel    = texture(sampler2D(albedoTexture, textureSampler), passUV).rgb;
    vec3 normalTexel    = texture(sampler2D(normalTexture, textureSampler), passUV).rgb;
    vec3 specularTexel  = texture(sampler2D(specularTexture, textureSampler), passUV).rgb;
    
    vec3 albedo = albedoTexel;
    
    vec3 T      = normalize(passTangent.xyz);
    vec3 N_geo  = normalize(passNormal);
    vec3 B      = cross(N_geo, T) * passTangent.w;
    mat3 TBN    = mat3(T, B, N_geo);
    normalTexel = normalTexel * 2 - 1;
    
    vec3 N          = TBN * normalTexel;
    vec3 sun        = lightInfo.sunStrength * lightInfo.sunColor * clamp(dot(N, lightInfo.L), 0, 1);
    sun             *= shadowTest(passPos, lightInfo, shadowMap, shadowMapSampler);
    vec3 ambient    = vec3(0.05);
	outColor        = albedo * (sun + ambient); 
}