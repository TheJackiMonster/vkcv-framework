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

vec3 voxelConeTrace(texture3D voxelTexture, sampler voxelSampler, vec3 direction, vec3 startPosition, float coneAngleRadian){

    int voxelResolution =  textureSize(sampler3D(voxelTexture, voxelSampler), 0).x;
    float voxelSize     = voxelInfo.extent / voxelResolution;
    float maxMip        = float(log2(voxelResolution));
    float maxStableMip  = 4;    // must be the same as in Voxelization::voxelizeMeshes
    maxMip              = min(maxMip, maxStableMip);
    float d             = sqrt(3 * pow(voxelSize, 2));
    vec3 color          = vec3(0);
    float a             = 0;
    
    float coneAngleHalf = coneAngleRadian * 0.5f;
    
    int maxSamples = 16;
    for(int i = 0; i < maxSamples; i++){
        
        vec3 samplePos      = startPosition + d * direction;
        vec3 sampleUV       = worldToVoxelCoordinates(samplePos, voxelInfo);
        
        if(a >= 0.95 || any(lessThan(sampleUV, vec3(0))) || any(greaterThan(sampleUV, vec3(1)))){
            break;
        }
        
        float coneDiameter  = 2 * tan(coneAngleHalf) * d;
        float mip           = log2(coneDiameter / voxelSize);
        mip                 = min(mip, maxMip);
    
        vec4 voxelSample    = textureLod(sampler3D(voxelTexture, voxelSampler), sampleUV , mip);
        
        color               += (1 - a) * voxelSample.rgb;
        voxelSample.a       = pow(voxelSample.a, 0.6);
        a                   += (1 - a) * voxelSample.a;
        
        d                   += coneDiameter;
        samplePos           = startPosition + d * direction;
        sampleUV            = worldToVoxelCoordinates(samplePos, voxelInfo);
    }
    return color;
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
    
    vec3 F_in       = fresnelSchlick(NoL, f0);
    vec3 F_out      = fresnelSchlick(NoV, f0);
    vec3 diffuse    = lambertBRDF(albedo) * (1 - F_in) * (1 - F_out);
    
    vec3 up         = N_geo.y >= 0.99 ? vec3(1, 0, 0) : vec3(0, 1, 0);
    vec3 right      = normalize(cross(up, N));
    up              = cross(N, right); 
    mat3 toSurface  = mat3(right, up, N);
    
    float coneAngle = 60.f / 180.f * pi;
    vec3 diffuseTrace = vec3(0);
    {
        vec3 sampleDirection    = toSurface * vec3(0, 0, 1);
        float weight            = pi / 4.f;
        diffuseTrace            += weight * voxelConeTrace(voxelTexture, voxelSampler, sampleDirection, passPos, coneAngle);
    }
    for(int i = 0; i < 6;i++){
        float theta             = 2 * pi / i;
        float phi               = pi / 3;   // 60 degrees
        vec3 sampleDirection    = toSurface * vec3(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
        float weight            = pi * (3.f / 4.f) / i;
        diffuseTrace            += voxelConeTrace(voxelTexture, voxelSampler, sampleDirection, passPos, coneAngle);
    }
    
	outColor = (diffuse + specular) * sun + diffuse * diffuseTrace;
    //outColor = diffuseTrace;
}