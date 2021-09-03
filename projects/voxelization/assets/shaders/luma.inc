#ifndef LUMA_INC
#define LUMA_INC

float computeLuma(vec3 c){
    return dot(c, vec3(0.21, 0.72, 0.07));
}

#endif // #ifndef LUMA_INC