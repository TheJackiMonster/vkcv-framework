#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "particleShading.inc"

layout(location = 0) in vec2 passTriangleCoordinates;
layout(location = 1) in vec3 passVelocity;
layout(location = 2) in float passlifeTime;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform uColor {
	vec4 color;
} Color;

layout(set=0,binding=1) uniform uPosition{
	vec2 position;
} Position;


void main()
{
	vec2 mouse = vec2(Position.position.x, Position.position.y);
    
    vec3 c0 = vec3(1, 1, 0.05);
    vec3 c1 = vec3(1, passlifeTime * 0.5, 0.05);
    vec3 c2 = vec3(passlifeTime * 0.5,passlifeTime * 0.5,0.05);
    vec3 c3 = vec3(1, 0.05, 0.05);
    
    if(passlifeTime  < 1){
        outColor = mix(c0, c1, passlifeTime );
    }
    else if(passlifeTime  < 2){
        outColor = mix(c1, c2, passlifeTime  - 1);
    }
    else{
        outColor = mix(c2, c3, clamp((passlifeTime  - 2) * 0.5, 0, 1));
    }
   
   // make the triangle look like a circle
   outColor *= circleFactor(passTriangleCoordinates);
   
   // fade out particle shortly before it dies
   outColor *= clamp(passlifeTime * 2, 0, 1);
}