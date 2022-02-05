#ifndef PARTICLE_INC
#define PARTICLE_INC

#define EPSILON 0.00000001f

struct ParticleMinimal {
    vec3 position;
    float size;
    vec3 velocity;
    float mass;
	
	vec3 pad;
	float weight_sum;
};

struct Particle {
    ParticleMinimal minimal;
    mat4 deformation;
	mat4 mls;
};

#define SIM_FORM_SPHERE 0
#define SIM_FORM_CUBE 1

#define SIM_TYPE_HYPERELASTIC 0
#define SIM_TYPE_FLUID 1

struct Simulation {
	float density;
	float size;
	float lame1;
	float lame2;
	int form;
	int type;
	float K;
	float E;
	float gamma;
};

const float PI = 3.1415926535897932384626433832795;

float sphere_volume(float radius) {
	return 4.0f * (radius * radius * radius) * PI / 3.0f;
}

float sphere_radius(float volume) {
	return pow(volume * 3.0f / 4.0f / PI, 1.0f / 3.0f);
}

float cube_volume(float radius) {
	return 8.0f * (radius * radius * radius);
}

float cube_radius(float volume) {
	return pow(volume / 8.0f, 1.0f / 3.0f);
}

float simulationMass(float density, float size, int form) {
	switch (form) {
		case SIM_FORM_SPHERE:
			return density * sphere_volume(size);
		case SIM_FORM_CUBE:
			return density * cube_volume(size);
		default:
			return 0.0f;
	}
}

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
	vec3 delta = abs(particle.position - voxel) / particle.size;
	
	if (any(isnan(delta)) || any(isinf(delta))) {
		return 0.0f;
	}
	
	vec3 weight = vec3(
		weight_C(delta.x),
		weight_C(delta.y),
		weight_C(delta.z)
	);
	
	float result = (
		weight.x * weight.y * weight.z
	) / particle.weight_sum;
	
	if ((isnan(result)) || (isinf(result))) {
		return 0.0f;
	} else {
		return result;
	}
}

#endif // PARTICLE_INC