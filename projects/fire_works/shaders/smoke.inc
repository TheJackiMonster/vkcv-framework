#ifndef SMOKE_INC
#define SMOKE_INC

struct smoke_t {
	vec3 position;
	float size;
	vec3 velocity;
	float scaling;
	vec3 color;
	uint eventID;
};

float smokeDensity(float size) {
	if (size > 0.0f) {
		return 0.025f / size;
	} else {
		return 0.0f;
	}
}

#endif // SMOKE_INC