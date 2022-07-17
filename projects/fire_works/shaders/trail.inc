#ifndef TRAIL_INC
#define TRAIL_INC

struct trail_t {
    uint particleIndex;
    uint startIndex;
    uint endIndex;
	uint useCount;
	vec3 color;
    float lifetime;
};

#endif // TRAIL_INC