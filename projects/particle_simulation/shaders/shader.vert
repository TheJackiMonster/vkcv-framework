#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 particle;

struct Particle
{
    vec3 position;
    float lifeTime;
    vec3 velocity;
    float padding_2;
};

layout(std430, binding = 2) coherent buffer buffer_inParticle
{
    Particle inParticle[];
};

layout( push_constant ) uniform constants{
    mat4 mvp;
};

layout(location = 1) out vec3 passVelocity;

void main()
{
    int id = gl_InstanceIndex;
    passVelocity = inParticle[id].velocity;
    vec3 moved_particle = particle + inParticle[id].position;
	gl_Position =   mvp * vec4(moved_particle, 1.0);
}