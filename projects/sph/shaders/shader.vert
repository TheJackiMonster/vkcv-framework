#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 particle;

struct Particle
{
    vec3 position;
    float padding;
    vec3 velocity;
    float density;
    vec3 force;
    float pressure;
};

layout(std430, binding = 2) readonly buffer buffer_inParticle1
{
    Particle inParticle1[];
};

layout(std430, binding = 3) readonly buffer buffer_inParticle2
{
    Particle inParticle2[];
};

layout( push_constant ) uniform constants{
    mat4 view;
    mat4 projection;
};

layout(location = 0) out vec2 passTriangleCoordinates;
layout(location = 1) out vec3 passVelocity;

void main()
{
    int id = gl_InstanceIndex;
    passVelocity = inParticle1[id].velocity;
    
    // particle position in view space
    vec4 positionView = view * vec4(inParticle1[id].position, 1);
    // by adding the triangle position in view space the mesh is always camera facing
    positionView.xyz += particle;
    // multiply with projection matrix for final position
	gl_Position = projection * positionView;
    
    // 0.01 corresponds to vertex position size in main
    float normalizationDivider  = 0.012;
    passTriangleCoordinates     = particle.xy / normalizationDivider;
}