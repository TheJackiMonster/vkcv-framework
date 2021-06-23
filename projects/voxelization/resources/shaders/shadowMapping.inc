#ifndef SHADOW_MAPPING_INC
#define SHADOW_MAPPING_INC

#include "lightInfo.inc"

// nice math blob from the moment shadow mapping presentation
float ComputeMSMShadowIntensity(vec4 _4Moments, float FragmentDepth, float DepthBias, float MomentBias)
{
    vec4 b=mix(_4Moments, vec4(0.5f,0.5f,0.5f,0.5f),MomentBias);
    vec3 z;
    z[0]=FragmentDepth-DepthBias;
    float L32D22=-b[0] * b[1] + b[2];
    float D22= -b[0] * b[0] + b[1];
    float SquaredDepthVariance=-b[1]*b[1]+b[3];
    float D33D22=dot(vec2(SquaredDepthVariance,-L32D22),
                     vec2(D22,                  L32D22));
                     
    float InvD22=1.0f/D22;
    float L32=L32D22*InvD22;
    vec3 c=vec3(1.0f,z[0],z[0]*z[0]);
    c[1]-=b.x;
    c[2]-=b.y+L32*c[1];
    c[1]*=InvD22;
    c[2]*=D22/D33D22;
    c[1]-=L32*c[2];
    c[0]-=dot(c.yz,b.xy);
    float p=c[1]/c[2];
    float q=c[0]/c[2];
    float r=sqrt((p*p*0.25f)-q);
    z[1]=-p*0.5f-r;
    z[2]=-p*0.5f+r;
    vec4 Switch=
    	(z[2]<z[0])?vec4(z[1],z[0],1.0f,1.0f):(
    	(z[1]<z[0])?vec4(z[0],z[1],0.0f,1.0f):
    	vec4(0.0f,0.0f,0.0f,0.0f));
    float Quotient=(Switch[0]*z[2]-b[0]*(Switch[0]+z[2])+b[1])
                  /((z[2]-Switch[1])*(z[0]-z[1]));
    return clamp(Switch[2]+Switch[3]*Quotient, 0, 1);
}

vec4 quantizeMoments(vec4 moments){
    mat4 T = mat4(
        -2.07224649f,     13.7948857237f,   0.105877704f,   9.7924062118f,
         32.23703778f,   -59.4683975703f, -1.9077466311f, -33.7652110555f,
        -68.571074599f,   82.0359750338f,  9.3496555107f,  47.9456096605f,
         39.3703274134f, -35.364903257f,  -6.6543490743f, -23.9728048165f);
    vec4 quantized = T * moments;
    quantized[0] += 0.0359558848;
    return quantized;
}

vec4 unquantizeMoments(vec4 moments){
    moments[0] -= 0.0359558848;
    mat4 T = mat4(
        0.2227744146f,  0.1549679261f,  0.1451988946f,  0.163127443f,
        0.0771972861f,  0.1394629426f,  0.2120202157f,  0.2591432266f,
        0.7926986636f,  0.7963415838f,  0.7258694464f,  0.6539092497f,
        0.0319417555f,  -0.1722823173f, -0.2758014811f, -0.3376131734f);
    return T * moments;
}

float shadowTest(vec3 worldPos, LightInfo lightInfo, texture2D shadowMap, sampler shadowMapSampler){
    vec4 lightPos = lightInfo.lightMatrix * vec4(worldPos, 1);
    lightPos /= lightPos.w;
    lightPos.xy = lightPos.xy * 0.5 + 0.5;
    
    if(any(lessThan(lightPos.xy, vec2(0))) || any(greaterThan(lightPos.xy, vec2(1)))){
        return 1;
    }
    
    lightPos.z = clamp(lightPos.z, 0, 1);

    vec4 shadowMapSample = texture(sampler2D(shadowMap, shadowMapSampler), lightPos.xy);
    
    shadowMapSample = unquantizeMoments(shadowMapSample);
    
    float depthBias     = 0.f;
    float momentBias    = 0.00003;
    
    return 1-ComputeMSMShadowIntensity(shadowMapSample, lightPos.z, depthBias, momentBias);
}

#endif // #ifndef SHADOW_MAPPING_INC