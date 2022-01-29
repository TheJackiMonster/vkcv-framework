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

float weight_A(float x) {
	return max(1.0f - x, 0.0f);
}

float weight_B(float x) {
	if (x < 0.5f) {
		return 0.75f - x * x;
	} else
	if (x < 1.5f) {
		float y = (1.5f - x);
		return 0.5f * y * y;
	} else {
		return 0.0f;
	}
}

float weight_C(float x) {
	if (x < 1.0f) {
		return (0.5f * x - 1.0f) * x*x + 2.0f / 3.0f;
	} else
	if (x < 2.0f) {
		float y = (2.0f - x);
		return 0.5f / 3.0f * y * y * y;
	} else {
		return 0.0f;
	}
}

float voxel_particle_weight(vec3 voxel, ParticleMinimal particle) {
	if (particle.size <= 0.0f) {
		return 0.0f;
	}
	
	vec3 delta = abs(particle.position - voxel) / particle.size;
	
	return weight_C(delta.x) * weight_C(delta.y) * weight_C(delta.z);
}

#endif // PARTICLE_INC