#ifndef PARTICLE_INC
#define PARTICLE_INC

struct particle_t {
	vec3 position;
	float lifetime;
	vec3 velocity;
	float size;
	vec3 color;
	float mass;
	vec3 pad0;
	uint eventId;
};

#endif // PARTICLE_INC