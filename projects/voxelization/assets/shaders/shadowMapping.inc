#ifndef SHADOW_MAPPING_INC
#define SHADOW_MAPPING_INC

#include "lightInfo.inc"

// nice math blob from the moment shadow mapping presentation
float ComputeMSMShadowIntensity(vec4 _4Moments, float FragmentDepth, float DepthBias, float MomentBias)
{
    vec4 b=mix(_4Moments, vec4(0, 0.63, 0, 0.63),MomentBias);
	
    vec3 z;
    z[0]=FragmentDepth-DepthBias;
    float L32D22=fma(-b[0], b[1], b[2]);
    float D22=fma(-b[0], b[0], b[1]);
    float SquaredDepthVariance=fma(-b[1], b[1], b[3]);
    float D33D22=dot(vec2(SquaredDepthVariance,-L32D22),
                     vec2(D22,                  L32D22));
                     
    float InvD22=1.0/D22;
    float L32=L32D22*InvD22;
    vec3 c=vec3(1.0,z[0],z[0]*z[0]);
    c[1]-=b.x;
    c[2]-=b.y+L32*c[1];
    c[1]*=InvD22;
    c[2]*=D22/D33D22;
    c[1]-=L32*c[2];
    c[0]-=dot(c.yz,b.xy);
    float p=c[1]/c[2];
    float q=c[0]/c[2];
    float r=sqrt((p*p*0.25)-q);
    z[1]=-p*0.5-r;
    z[2]=-p*0.5+r;
    vec4 Switch=
    	(z[2]<z[0])?vec4(z[1],z[0],1.0,1.0):(
    	(z[1]<z[0])?vec4(z[0],z[1],0.0,1.0):
    	vec4(0.0));
    float Quotient=(Switch[0]*z[2]-b[0]*(Switch[0]+z[2])+b[1])
                  /((z[2]-Switch[1])*(z[0]-z[1]));
    return 1-clamp(Switch[2]+Switch[3]*Quotient, 0, 1);
}

vec4 quantizeMoments(vec4 moments){
    vec4 quantized;
	quantized.r = 1.5 * moments.r - 2 * moments.b + 0.5;
	quantized.g = 4   * moments.g - 4 * moments.a;
	quantized.b = sqrt(3)/2 * moments.r - sqrt(12)/9 * moments.b + 0.5;
	quantized.a = 0.5 * moments.g + 0.5 * moments.a;
	
	return quantized;
}

vec4 unquantizeMoments(vec4 moments){
    moments -= vec4(0.5, 0, 0.5, 0);
	vec4 unquantized;
	unquantized.r = -1.f / 3 * moments.r + sqrt(3) * moments.b;
	unquantized.g = 0.125 * moments.g + moments.a;
	unquantized.b = -0.75 * moments.r + 0.75 * sqrt(3) * moments.b;
	unquantized.a = -0.125 * moments.g + moments.a;
	return unquantized * 0.98;
}

float rescaleRange(float a, float b, float v)
{
    return clamp((v - a) / (b - a), 0, 1);
}

float reduceLightBleeding(float shadow, float amount)
{
   return rescaleRange(amount, 1.0f, shadow);
}

float shadowTest(vec3 worldPos, LightInfo lightInfo, texture2D shadowMap, sampler shadowMapSampler, vec2 offset){
    vec4 lightPos   = lightInfo.lightMatrix * vec4(worldPos, 1);
    lightPos        /= lightPos.w;
    lightPos.xy     = lightPos.xy * 0.5 + 0.5;
    lightPos.xy     += offset;
    
    if(any(lessThan(lightPos.xy, vec2(0))) || any(greaterThan(lightPos.xy, vec2(1)))){
        return 1;
    }
	
    lightPos.z = clamp(lightPos.z, 0, 1);
	lightPos.z = 2 * lightPos.z - 1;	// algorithm expects depth in range [-1:1]

    vec4 shadowMapSample = texture(sampler2D(shadowMap, shadowMapSampler), lightPos.xy);
    
    shadowMapSample = unquantizeMoments(shadowMapSample);
    
    float depthBias     = 0.f;
    float momentBias    = 0.0006;
    
    float shadow = ComputeMSMShadowIntensity(shadowMapSample, lightPos.z, depthBias, momentBias);
	return clamp(shadow, 0, 1);
    return reduceLightBleeding(shadow, 0.1f);
}

#endif // #ifndef SHADOW_MAPPING_INC