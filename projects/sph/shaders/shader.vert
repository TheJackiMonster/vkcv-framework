#version 460 core
#extension GL_ARB_separate_shader_objects : enable

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
    const float particle_size = 0.02f;

    vec2 positions[3] = {
        vec2(-particle_size, particle_size),
        vec2(particle_size, particle_size),
        vec2(0.0f, -particle_size)
    };

    int id = gl_InstanceIndex;
    passVelocity = inParticle1[id].velocity;
    
    // particle position in view space
    vec4 positionView = view * vec4(inParticle1[id].position, 1);
    // by adding the triangle position in view space the mesh is always camera facing
    positionView.xyz += vec3(positions[gl_VertexIndex], 0.0f);
    // multiply with projection matrix for final position
	gl_Position = projection * positionView;
    
    // 0.01 corresponds to vertex position size in main
    float normalizationDivider  = particle_size;
    passTriangleCoordinates     = positions[gl_VertexIndex] / normalizationDivider;
}