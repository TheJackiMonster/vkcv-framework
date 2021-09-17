#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

#define M_PI 3.1415926535897932384626433832795

//Mat struct
/*struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 emission;
};*/

//hitAttributeEXT vec2 hitCoordinate;

layout(location = 0) rayPayloadInEXT Payload {
  vec3 rayOrigin;
  vec3 rayDirection;
  vec3 previousNormal;

  vec3 directColor;
  vec3 indirectColor;
  int rayDepth;

  int rayActive;
} payload;

//layout(location = 1) rayPayloadEXT bool isShadow;

layout(binding = 2, set = 0) uniform accelerationStructureEXT tlas;     // top level acceleration structure (for the noobs here (you!))

layout( push_constant ) uniform constants {
    vec4 camera_position;   // as origin for ray generation
    vec4 camera_right;      // for computing ray direction
    vec4 camera_up;         // for computing ray direction
    vec4 camera_forward;    // for computing ray direction

    uint frameCount;        // what is this? the actual frame?
}camera;


layout(binding = 3, set = 0) buffer rtxVertices
{
    float vertices[];
} rtxVertexBuffer;

layout(binding = 4, set = 0) buffer rtxIndices
{
    uint indices[];
} rtxIndexBuffer;

/*
layout(binding = 0, set = 1) buffer MaterialIndexBuffer { uint data[]; } materialIndexBuffer;
layout(binding = 1, set = 1) buffer MaterialBuffer { Material data[]; } materialBuffer;
*/

float random(vec2 uv, float seed) {
  return fract(sin(mod(dot(uv, vec2(12.9898, 78.233)) + 1113.1 * seed, M_PI)) * 43758.5453);;
}

vec3 uniformSampleHemisphere(vec2 uv) {
  float z = uv.x;
  float r = sqrt(max(0, 1.0 - z * z));
  float phi = 2.0 * M_PI * uv.y;

  return vec3(r * cos(phi), z, r * sin(phi));
}

vec3 alignHemisphereWithCoordinateSystem(vec3 hemisphere, vec3 up) {
  vec3 right = normalize(cross(up, vec3(0.0072f, 1.0f, 0.0034f)));
  vec3 forward = cross(right, up);

  return hemisphere.x * right + hemisphere.y * up + hemisphere.z * forward;
}

void main() {
    if (payload.rayActive == 0) {
        return;
    }
    
    //ivec3 rtindices = ivec3(rtxIndexBuffer.indices[3 * gl_PrimitiveID + 0], rtxIndexBuffer.indices[3 * gl_PrimitiveID + 1], rtxIndexBuffer.indices[3 * gl_PrimitiveID + 2]);
    /*
    vec3 barycentric = vec3(1.0 - hitCoordinate.x - hitCoordinate.y, hitCoordinate.x, hitCoordinate.y);

    vec3 vertexA = vec3(rtxVertexBuffer.vertices[3 * rtindices.x + 0], rtxVertexBuffer.vertices[3 * rtindices.x + 1], rtxVertexBuffer.vertices[3 * rtindices.x + 2]);
    vec3 vertexB = vec3(rtxVertexBuffer.vertices[3 * rtindices.y + 0], rtxVertexBuffer.vertices[3 * rtindices.y + 1], rtxVertexBuffer.vertices[3 * rtindices.y + 2]);
    vec3 vertexC = vec3(rtxVertexBuffer.vertices[3 * rtindices.z + 0], rtxVertexBuffer.vertices[3 * rtindices.z + 1], rtxVertexBuffer.vertices[3 * rtindices.z + 2]);

    vec3 position = vertexA * barycentric.x + vertexB * barycentric.y + vertexC * barycentric.z;
    vec3 geometricNormal = normalize(cross(vertexB - vertexA, vertexC - vertexA));*/

    payload.directColor=vec3(1,0,0);    
}
