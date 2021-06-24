#ifndef SHADOW_MAPPING_INC
#define SHADOW_MAPPING_INC

#include "lightInfo.inc"

// nice math blob from the moment shadow mapping presentation
float ComputeMSMShadowIntensity(vec4 _4Moments, float FragmentDepth, float DepthBias, float MomentBias)
{
    vec4 b=mix(_4Moments, vec4(0.5),MomentBias);
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
    mat4 T = mat4(
        -2.07224649,     13.7948857237,   0.105877704,   9.7924062118,
         32.23703778,   -59.4683975703, -1.9077466311, -33.7652110555,
        -68.571074599,   82.0359750338,  9.3496555107,  47.9456096605,
         39.3703274134, -35.364903257,  -6.6543490743, -23.9728048165);
    vec4 quantized = T * moments;
    quantized[0] += 0.0359558848;
    return quantized;
}

vec4 unquantizeMoments(vec4 moments){
    moments[0] -= 0.0359558848;
    mat4 T = mat4(
        0.2227744146,  0.1549679261,  0.1451988946,  0.163127443,
        0.0771972861,  0.1394629426,  0.2120202157,  0.2591432266,
        0.7926986636,  0.7963415838,  0.7258694464,  0.6539092497,
        0.0319417555,  -0.1722823173, -0.2758014811, -0.3376131734);
    return T * moments;
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
    vec4 lightPos = lightInfo.lightMatrix * vec4(worldPos, 1);
    lightPos /= lightPos.w;
    lightPos.xy = lightPos.xy * 0.5 + 0.5;
    lightPos.xy += offset;
    
    if(any(lessThan(lightPos.xy, vec2(0))) || any(greaterThan(lightPos.xy, vec2(1)))){
        return 1;
    }
    
    lightPos.z = clamp(lightPos.z, 0, 1);

    vec4 shadowMapSample = texture(sampler2D(shadowMap, shadowMapSampler), lightPos.xy);
    
    shadowMapSample = unquantizeMoments(shadowMapSample);
    
    float depthBias     = 0.f;
    float momentBias    = 0.000002;
    
    float shadow = ComputeMSMShadowIntensity(shadowMapSample, lightPos.z, depthBias, momentBias);
    return reduceLightBleeding(shadow, 0.1f);
}

#endif // #ifndef SHADOW_MAPPING_INC