#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "perMeshResources.inc"
#include "lightInfo.inc"
#include "shadowMapping.inc"
#include "brdf.inc"
#include "voxel.inc"

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

layout(set=0, binding=4) uniform texture3D  voxelTexture;
layout(set=0, binding=5) uniform sampler    voxelSampler;

layout(set=0, binding=6) uniform VoxelInfoBuffer{
    VoxelInfo voxelInfo;
};

vec3 cookTorrance(vec3 f0, float r, vec3 N, vec3 V, vec3 L){
    
    vec3 H  = normalize(L + V);
    
    float NoH = clamp(dot(N, H), 0, 1);
    float NoL = clamp(dot(N, L), 0, 1);
    float NoV = clamp(dot(N, V), 0, 1);
    
    vec3    F           = fresnelSchlick(NoH, f0);
    float   D           = GGXDistribution(r, NoH);
    float   G           = GGXSmithShadowing(r, NoV, NoL);
    return (F * D * G) / max(4 * NoV * NoL, 0.000001);
}

float roughnessToConeAngle(float r){
    return mix(degreeToRadian(10), degreeToRadian(90), r);
}

void main()	{

    vec3 albedoTexel    = texture(sampler2D(albedoTexture, textureSampler), passUV).rgb;
    vec3 normalTexel    = texture(sampler2D(normalTexture, textureSampler), passUV).rgb;
    vec3 specularTexel  = texture(sampler2D(specularTexture, textureSampler), passUV).rgb;
    
    float r             = specularTexel.g;
    
    float metal         = specularTexel.b;
    vec3 albedo         = mix(albedoTexel, vec3(0), metal);
    vec3 f0_dielectric  = vec3(0.04f);
    vec3 f0             = mix(f0_dielectric, albedoTexel, metal);
    
    vec3 T      = normalize(passTangent.xyz);
    vec3 N_geo  = normalize(passNormal);
    vec3 B      = cross(N_geo, T) * passTangent.w;
    mat3 TBN    = mat3(T, B, N_geo);
    normalTexel = normalTexel * 2 - 1;
    
    vec3 N  = TBN * normalTexel;
    vec3 L  = lightInfo.L;
    vec3 V  = normalize(cameraPos - passPos);
    
    float NoL = clamp(dot(N, L), 0, 1);    
    float NoV = clamp(dot(N, V), 0, 1);
    
    vec3 sunSpecular = cookTorrance(f0, r, N, V, L);
    
    vec3 sun        = lightInfo.sunStrength * lightInfo.sunColor * NoL;
    sun             *= shadowTest(passPos, lightInfo, shadowMap, shadowMapSampler);
    
    vec3 F_in       = fresnelSchlick(NoL, f0);
    vec3 F_out      = fresnelSchlick(NoV, f0);
    vec3 diffuse    = lambertBRDF(albedo) * (1 - F_in) * (1 - F_out);
    
    vec3 up         = N_geo.y >= 0.99 ? vec3(1, 0, 0) : vec3(0, 1, 0);
    vec3 right      = normalize(cross(up, N));
    up              = cross(N, right); 
    mat3 toSurface  = mat3(right, up, N);
    
    vec3 diffuseTrace = diffuseVoxelTraceHemisphere(toSurface, passPos, voxelTexture, voxelSampler, voxelInfo);
    
    vec3 R                      = reflect(-V, N);
    float reflectionConeAngle   = degreeToRadian(roughnessToConeAngle(r));
    vec3 offsetTraceStart       = passPos + N_geo * 0.1f;
    vec3 specularTrace          = voxelConeTrace(R, offsetTraceStart, reflectionConeAngle, voxelTexture, voxelSampler, voxelInfo);
    vec3 reflectionBRDF         = cookTorrance(f0, r, N, V, R);
    
	outColor = 
        (diffuse + sunSpecular) * sun + 
        lambertBRDF(albedo) * diffuseTrace + 
        reflectionBRDF * specularTrace;
}