#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_16bit_storage : require

hitAttributeEXT vec2 attributes;

layout(location = 0) rayPayloadInEXT Payload {
  float hitSky;
  vec3 worldPosition;
  vec3 worldNormal;
  uvec4 hit;
} payload;

layout(binding = 2, set = 0, scalar) buffer rtVertices
{
    float vertices[];
};

layout(binding = 3, set = 0, scalar) buffer rtIndices
{
    uint16_t indices[];
};

void main() {
    payload.worldPosition = vec3(1.0, 0.0, 0.5);

    payload.hit = uvec4(
        gl_PrimitiveID,
        gl_InstanceID,
        gl_InstanceCustomIndexEXT,
        gl_GeometryIndexEXT
    );

    ivec3 indicesVec = ivec3(indices[3 * gl_PrimitiveID + 0], indices[3 * gl_PrimitiveID + 1], indices[3 * gl_PrimitiveID + 2]);

    // current triangle
    const vec3 v0 = vec3(vertices[3 * indicesVec.x + 0],vertices[3 * indicesVec.x + 1],vertices[3 * indicesVec.x + 2]);
    const vec3 v1 = vec3(vertices[3 * indicesVec.y + 0],vertices[3 * indicesVec.y + 1],vertices[3 * indicesVec.y + 2]);
    const vec3 v2 = vec3(vertices[3 * indicesVec.z + 0],vertices[3 * indicesVec.z + 1],vertices[3 * indicesVec.z + 2]);

    // use barycentric coordinates to compute intersection
    const vec3 barycentrics = vec3(1.0 - attributes.x - attributes.y, attributes.xy);
    const vec3 objectPosition = v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;

    payload.worldPosition = gl_ObjectToWorldEXT * vec4(objectPosition, 1.0);

    const vec3 objectNormal = cross(v1 - v0, v2 - v0);

    payload.worldNormal = normalize((objectNormal * gl_WorldToObjectEXT).xyz);
    payload.worldNormal = faceforward(payload.worldNormal, gl_WorldRayDirectionEXT, payload.worldNormal);

    payload.hitSky = 0.0f;
}
