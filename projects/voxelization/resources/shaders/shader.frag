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

layout(set=0, binding=3) uniform cameraBuffer {
    vec3 cameraPos;
};

const float pi = 3.1415; 

vec3 lambertBRDF(vec3 albedo){
    return albedo / pi;
}

vec3 fresnelSchlick(float cosTheta, vec3 f0){
    return f0 + (vec3(1) - f0) * pow(1 - cosTheta, 5);
}

float GGXDistribution(float r, float NoH){
    float r2    = r * r;
    float denom = pi * pow(NoH * NoH * (r2 - 1) + 1, 2);
    return r2 / max(denom, 0.000001);
}

float GGXSmithShadowingPart(float r, float cosTheta){
    float nom   = cosTheta * 2;
    float r2    = r * r;
    float denom = cosTheta + sqrt(r2 + (1 - r2) * cosTheta * cosTheta);
    return nom / max(denom, 0.000001);
}

float GGXSmithShadowing(float r, float NoV, float NoL){
    return GGXSmithShadowingPart(r, NoV) * GGXSmithShadowingPart(r, NoL);
}

void main()	{

    vec3 albedoTexel    = texture(sampler2D(albedoTexture, textureSampler), passUV).rgb;
    vec3 normalTexel    = texture(sampler2D(normalTexture, textureSampler), passUV).rgb;
    vec3 specularTexel  = texture(sampler2D(specularTexture, textureSampler), passUV).rgb;
    
    float r             = specularTexel.g;
    r                   *= r;   // remapping roughness for perceptual linearity
    
    float metal         = specularTexel.b;
    vec3 albedo         = mix(albedoTexel, vec3(0), metal);
    vec3 f0_dielectric  = vec3(0.04f);
    vec3 f0             = mix(f0_dielectric, albedo, metal);
    
    vec3 T      = normalize(passTangent.xyz);
    vec3 N_geo  = normalize(passNormal);
    vec3 B      = cross(N_geo, T) * passTangent.w;
    mat3 TBN    = mat3(T, B, N_geo);
    normalTexel = normalTexel * 2 - 1;
    
    vec3 N  = TBN * normalTexel;
    vec3 L  = lightInfo.L;
    vec3 V  = normalize(cameraPos - passPos);
    vec3 H  = normalize(L + V);
    
    float NoH = clamp(dot(N, H), 0, 1);
    float NoL = clamp(dot(N, L), 0, 1);
    float NoV = clamp(dot(N, V), 0, 1);
    
    vec3 F          = fresnelSchlick(NoH, f0);
    float D         = GGXDistribution(r, NoH);
    float G         = GGXSmithShadowing(r, NoV, NoL);
    vec3 specular   = (F * D * G) / max(4 * NoV * NoL, 0.000001);
    
    vec3 sun        = lightInfo.sunStrength * lightInfo.sunColor * NoL;
    sun             *= shadowTest(passPos, lightInfo, shadowMap, shadowMapSampler);
    vec3 ambient    = vec3(0.05);
    
    vec3 F_in       = fresnelSchlick(NoL, f0);
    vec3 F_out      = fresnelSchlick(NoV, f0);
    vec3 diffuse    = lambertBRDF(albedo) * (1 - F_in) * (1 - F_out);
    
	outColor        = (diffuse + specular) * sun + lambertBRDF(albedo) * ambient;
}