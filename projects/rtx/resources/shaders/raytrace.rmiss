#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT Payload {
  vec3 rayOrigin;
  vec3 rayDirection;
  vec3 previousNormal;

  vec3 directColor;
  vec3 indirectColor;
  int rayDepth;

  int rayActive;
} payload;

layout(binding = 5, set = 0) uniform accelerationStructureEXT tlas; //not neccesary in shader but for compiling ->bug

void main() {
    payload.rayActive = 0;
}
