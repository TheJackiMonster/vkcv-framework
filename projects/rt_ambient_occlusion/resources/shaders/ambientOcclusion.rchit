#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_16bit_storage : require

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

hitAttributeEXT vec2 attributes;

layout(location = 0) rayPayloadInEXT Payload {
  float hitSky;
  vec3 worldPosition;
  vec3 worldNormal;
} payload;

layout(buffer_reference, scalar) buffer Vertices {
    float v[];
};

layout(buffer_reference, scalar) buffer Indices {
    uint16_t i[];
    //uint i[];
};

struct ObjDesc {
    uint64_t vertexAddress;
    uint64_t indexAddress;
    uint vertexStride;
    uint pad0;
    uint pad1;
    uint pad2;
    mat4 transform;
};

layout(binding = 2, set = 0, scalar) buffer rtObjects {
    ObjDesc objects[];
};

layout(binding = 3, set = 0, scalar) buffer rtContext {
    uint instanceCount;
    uint sampleCount;
};

void main() {
    int instanceIndex = gl_InstanceID + gl_GeometryIndexEXT * int(instanceCount);

    if (instanceIndex >= objects.length()) {
        payload.hitSky = 1.0f;
        return;
    }

    ObjDesc obj = objects[nonuniformEXT(instanceIndex)];
    Indices indices = Indices(obj.indexAddress);
    Vertices vertices = Vertices(obj.vertexAddress);

    const uint stride = obj.vertexStride;

    ivec3 indicesVec = ivec3(
        indices.i[3 * gl_PrimitiveID + 0],
        indices.i[3 * gl_PrimitiveID + 1],
        indices.i[3 * gl_PrimitiveID + 2]
    );

    // current triangle
    const vec3 v0 = vec3(vertices.v[stride * indicesVec.x + 0], vertices.v[stride * indicesVec.x + 1], vertices.v[stride * indicesVec.x + 2]);
    const vec3 v1 = vec3(vertices.v[stride * indicesVec.y + 0], vertices.v[stride * indicesVec.y + 1], vertices.v[stride * indicesVec.y + 2]);
    const vec3 v2 = vec3(vertices.v[stride * indicesVec.z + 0], vertices.v[stride * indicesVec.z + 1], vertices.v[stride * indicesVec.z + 2]);

    // use barycentric coordinates to compute intersection
    const vec3 barycentrics = vec3(1.0 - attributes.x - attributes.y, attributes.xy);
    const vec3 objectPosition = v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;

    payload.worldPosition = vec3(gl_ObjectToWorldEXT * obj.transform * vec4(objectPosition, 1));

    const vec3 objectNormal = cross(v1 - v0, v2 - v0);
    const vec3 worldNormal = normalize(vec3(objectNormal * gl_WorldToObjectEXT * obj.transform));

    payload.worldNormal = worldNormal;
    payload.hitSky = 0.0f;
}
