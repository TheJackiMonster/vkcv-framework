#ifndef PARTICLE_INC
#define PARTICLE_INC

struct ParticleMinimal {
    vec3 position;
    float size;
    vec3 velocity;
    float mass;
};

struct Particle {
    ParticleMinimal minimal;
    mat4 deformation;
};

#endif // PARTICLE_INC