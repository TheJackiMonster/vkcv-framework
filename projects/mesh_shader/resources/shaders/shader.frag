#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in  vec3 passNormal;
layout(location = 1) in  flat uint passTaskIndex;
layout(location = 0) out vec3 outColor;

uint lowbias32(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float hashToFloat(uint hash){
    return (hash % 255) / 255.f;
}

vec3 colorFromIndex(uint i){
    return vec3(
        hashToFloat(lowbias32(i+0)),
        hashToFloat(lowbias32(i+1)),
        hashToFloat(lowbias32(i+2)));
}

void main() {
	outColor = normalize(passNormal) * 0.5 + 0.5;
    outColor = colorFromIndex(passTaskIndex);
}