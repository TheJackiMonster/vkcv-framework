#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT Payload {
  float hitSky;
  vec3 worldPosition;
  vec3 worldNormal;
} payload;

void main() {
    payload.hitSky = 1.0f;
}
