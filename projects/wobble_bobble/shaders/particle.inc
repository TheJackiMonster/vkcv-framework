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

const float PI = 3.1415926535897932384626433832795;

float sphere_volume(float radius) {
	return 4.0f * (radius * radius * radius) * PI / 3.0f;
}

float sphere_radius(float volume) {
	return pow(volume * 3.0f / 4.0f / PI, 1.0f / 3.0f);
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
	if (particle.size <= 0.0f) {
		return 0.0f;
	}
	
	vec3 delta = abs(particle.position - voxel) / particle.size;
	
	vec3 weight = vec3(
		weight_C(delta.x),
		weight_C(delta.y),
		weight_C(delta.z)
	);
	
	return weight.x * weight.y * weight.z;
}

float grad_weight_A(float x) {
	return -1.0f;
}

float grad_weight_B(float x) {
	if (x < 0.5f) {
		return -2.0f * x;
	} else
	if (x < 1.5f) {
		return -1.5f + x;
	} else {
		return 0.0f;
	}
}

float grad_weight_C(float x) {
	if (x < 1.0f) {
		return 1.5f * x * x - 2.0f * x;
	} else
	if (x < 2.0f) {
		float y = (2.0f - x);
		return -0.5f * y * y;
	} else {
		return 0.0f;
	}
}

vec3 voxel_particle_grad_weight(vec3 voxel, ParticleMinimal particle) {
	if (particle.size <= 0.0f) {
		return vec3(0.0f);
	}
	
	vec3 sign_delta = (particle.position - voxel) / particle.size;
	vec3 delta = abs(sign_delta);
	vec3 sign = sign(sign_delta);
	
	vec3 weight = vec3(
		weight_C(delta.x),
		weight_C(delta.y),
		weight_C(delta.z)
	);
	
	vec3 grad_weight = vec3(
		grad_weight_C(delta.x),
		grad_weight_C(delta.y),
		grad_weight_C(delta.z)
	) * sign;
	
	return vec3(
			grad_weight.x * weight.y * weight.z,
			grad_weight.y * weight.z * weight.x,
			grad_weight.z * weight.x * weight.y
	);
}

#endif // PARTICLE_INC