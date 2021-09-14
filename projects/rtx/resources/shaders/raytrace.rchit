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


layout(binding = 3, set = 0) buffer rtxVertices
{
    vec3 vertices[];
};

layout(binding = 4, set = 0) buffer rtxIndices
{
    uint indices[];
};

void main() {
    payload.directColor=vec3(1,0,0);    
}
