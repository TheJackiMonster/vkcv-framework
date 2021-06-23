#ifndef LIGHT_INFO_INC
#define LIGHT_INFO_INC

struct LightInfo{
    vec3 L;             float padding;
    vec3 sunColor;      
    float sunStrength;
    mat4 lightMatrix;
    vec2 warps;
};

#endif // #ifndef LIGHT_INFO_INC