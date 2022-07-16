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

#endif // SMOKE_INC