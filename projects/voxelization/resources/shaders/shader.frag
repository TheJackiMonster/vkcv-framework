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

layout(set=0, binding=7) uniform VolumetricSettings {
    vec3    scatteringCoefficient;
    float   volumetricAmbientLight;
    vec3    absorptionCoefficient;
};


vec3 cookTorrance(vec3 f0, float r, vec3 N, vec3 V, vec3 L){
    
    vec3 H  = normalize(L + V);
    
    float NoH = clamp(dot(N, H), 0, 1);
    float NoL = clamp(dot(N, L), 0, 1);
    float NoV = clamp(abs(dot(N, V)), 0, 1);    // abs to account for wrong visibility caused by normal mapping
    
    vec3    F = fresnelSchlick(NoH, f0);
    float   D = GGXDistribution(r, NoH);
    float   G = GGXSmithShadowing(r, NoV, NoL);
    
    return (F * D * G) / max(4 * NoV * NoL, 0.00001);
}

float roughnessToConeAngleDegree(float r){
    return mix(degreeToRadian(3), degreeToRadian(60), r);
}

// from: "Next Generation Post Processing in Call Of Duty Advanced Warfare" slide page 123
float interleavedGradientNoise(vec2 uv){
    vec3 magic = vec3(0.06711056, 0.00583715, 62.9829189);
    return fract(magic.z * fract(dot(uv, magic.xy)));
}

// from: https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
vec3 EnvBRDFApprox(vec3 SpecularColor, float Roughness, float NoV )
{
	const vec4 c0 = { -1, -0.0275, -0.572, 0.022 };
	const vec4 c1 = { 1, 0.0425, 1.04, -0.04 };
	vec4 r = Roughness * c0 + c1;
	float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
	vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}

float isotropicPhase(){
    return 1 / pi;
}

vec3 volumetricLighting(vec3 colorIn, vec3 V, vec3 pos, float d){
    vec3 color      = colorIn;
    
    int sampleCount = 48;
    float stepSize  = d / sampleCount;
    
    vec3 extinctionCoefficient = scatteringCoefficient + absorptionCoefficient;
    
    float noiseScale    = 0.1;
    pos                 += V * noiseScale * interleavedGradientNoise(gl_FragCoord.xy);
    
    for(int i = 0; i < sampleCount; i++){
        vec3    samplePoint = pos + V * i * stepSize;
        float   phase       = isotropicPhase();
        vec3    light       = lightInfo.sunColor * lightInfo.sunStrength;
        float   shadow      = shadowTest(samplePoint, lightInfo, shadowMap, shadowMapSampler, vec2(0));
        light               *= shadow;
        light               += volumetricAmbientLight;
        
        color               += phase * light * scatteringCoefficient * stepSize;
        color               *= exp(-stepSize * extinctionCoefficient);
    }
    return color;
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

    vec3 N  = normalize(TBN * normalTexel);
    vec3 L  = lightInfo.L;
    vec3 V  = normalize(cameraPos - passPos);
    
    float NoL = clamp(dot(N, L), 0, 1);    
    float NoV = clamp(abs(dot(N, V)), 0, 1);
    
    vec3 sunSpecular    = cookTorrance(f0, r, N, V, L);
    vec3 sun            = lightInfo.sunStrength * lightInfo.sunColor * NoL;
    
    float   noise           = 2 * pi * interleavedGradientNoise(gl_FragCoord.xy);
    vec2    shadowOffset    = 0.05f * vec2(sin(noise), cos(noise)) / textureSize(sampler2D(shadowMap, shadowMapSampler), 0);
    float   shadow          = shadowTest(passPos, lightInfo, shadowMap, shadowMapSampler, shadowOffset);
    sun                     *= shadow;
    
    vec3 F_in       = fresnelSchlick(NoL, f0);
    vec3 F_out      = fresnelSchlick(NoV, f0);
    vec3 diffuse    = lambertBRDF(albedo) * (1 - F_in) * (1 - F_out);
    
    vec3 up         = abs(N_geo.y) >= 0.99 ? vec3(1, 0, 0) : vec3(0, 1, 0);
    vec3 right      = normalize(cross(up, N));
    up              = cross(N, right); 
    mat3 toSurface  = mat3(right, up, N);
    
    vec3 diffuseTrace = diffuseVoxelTraceHemisphere(toSurface, passPos, voxelTexture, voxelSampler, voxelInfo);
    
    vec3 R                      = reflect(-V, N);
    float reflectionConeAngle   = roughnessToConeAngleDegree(r);
    vec3 offsetTraceStart       = passPos + N_geo * 0.1f;
    offsetTraceStart            += R * interleavedGradientNoise(gl_FragCoord.xy) * 0.5;
    vec3 specularTrace          = voxelConeTrace(R, offsetTraceStart, reflectionConeAngle, voxelTexture, voxelSampler, voxelInfo);
    specularTrace               *= clamp(dot(N, R), 0, 1);
    vec3 reflectionBRDF         = EnvBRDFApprox(f0, r, NoV); 
    
	outColor = 
        (diffuse + sunSpecular) * sun + 
        lambertBRDF(albedo) * diffuseTrace + 
        reflectionBRDF * specularTrace;
        
    float d     = distance(cameraPos, passPos);
    outColor    = volumetricLighting(outColor, V, passPos, d);
}