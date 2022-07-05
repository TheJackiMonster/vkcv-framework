#ifndef EVENT_INC
#define EVENT_INC

struct event_t {
	vec3 direction;
	float startTime;
	vec3 color;
	float velocity;
	
	uint count;
	uint index;
	uint parent;
	uint pad0;
	
	float lifetime;
	float mass;
	float size;
	float pad1;
};

#endif // EVENT_INC