
struct Task {
    uint meshletIndices[32];
    mat4 mvp;
};

struct Meshlet {
    uint  vertexOffset;
    uint  vertexCount;
    uint  indexOffset;
    uint  indexCount;
    vec3  meanPosition;
    float boundingSphereRadius;
};
