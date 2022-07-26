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

vec4 smokeBlend(vec4 dst, vec4 src) {
	const float f = max(1.0f - dst.a, 0.0f);
	const float a = clamp(0.0f, 1.0f, src.a);

	return vec4(
		dst.rgb + src.rgb * a * f,
		dst.a + a * f
	);
}

#endif // SMOKE_INC