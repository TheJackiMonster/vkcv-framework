#ifndef BRDF_INC
#define BRDF_INC

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

#endif // #ifndef BRDF_INC