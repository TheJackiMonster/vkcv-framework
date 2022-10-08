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
	uint continuous;
	
	float lifetime;
	float mass;
	float size;
	uint contCount;
};

#endif // EVENT_INC