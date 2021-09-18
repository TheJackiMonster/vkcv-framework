#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

#define M_PI 3.1415926535897932384626433832795

//Mat struct
struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 emission;
};

struct Light {
	vec3 position;
	float intensity;
};


hitAttributeEXT vec2 hitCoordinate;

layout(location = 0) rayPayloadInEXT Payload {
  vec3 rayOrigin;
  vec3 rayDirection;
  vec3 previousNormal;

  vec3 directColor;
  vec3 indirectColor;
  int rayDepth;

  int rayActive;
} payload;

layout(location = 1) rayPayloadEXT bool isShadow;

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
    Material ivory = {vec3(1,0,0), vec3(0.4, 0.4, 0.3), vec3(50.,50.,50.), vec3(0.6, 0.3, 0.1)};
    Material mirror  = {vec3(1,0,0), vec3(1.0, 1.0, 1.0), vec3(1425.,1425.,1425.), vec3(0.0, 10.0, 0.8)};
    Light rtxLight1 = {vec3(5, 5, 5), 1.5};
    Light rtxLight2 = {vec3(-5,  -5, -2.5), 3};
    Light rtxLight3 = {vec3(-5,  -5,  5), 1.7};

    ivec3 rtindices = ivec3(rtxIndexBuffer.indices[3 * gl_PrimitiveID + 0], rtxIndexBuffer.indices[3 * gl_PrimitiveID + 1], rtxIndexBuffer.indices[3 * gl_PrimitiveID + 2]);

    vec3 barycentric = vec3(1.0 - hitCoordinate.x - hitCoordinate.y, hitCoordinate.x, hitCoordinate.y);

    vec3 vertexA = vec3(rtxVertexBuffer.vertices[3 * rtindices.x + 0], rtxVertexBuffer.vertices[3 * rtindices.x + 1], rtxVertexBuffer.vertices[3 * rtindices.x + 2]);
    vec3 vertexB = vec3(rtxVertexBuffer.vertices[3 * rtindices.y + 0], rtxVertexBuffer.vertices[3 * rtindices.y + 1], rtxVertexBuffer.vertices[3 * rtindices.y + 2]);
    vec3 vertexC = vec3(rtxVertexBuffer.vertices[3 * rtindices.z + 0], rtxVertexBuffer.vertices[3 * rtindices.z + 1], rtxVertexBuffer.vertices[3 * rtindices.z + 2]);

    vec3 position = vertexA * barycentric.x + vertexB * barycentric.y + vertexC * barycentric.z;
    vec3 geometricNormal = normalize(cross(vertexB - vertexA, vertexC - vertexA));

    //vec3 surfaceColor = materialBuffer.data[materialIndexBuffer.data[gl_PrimitiveID]].diffuse;
    vec3 surfaceColor = ivory.diffuse;

    if(payload.rayOrigin == rtxLight1.position || payload.rayOrigin == rtxLight2.position || payload.rayOrigin == rtxLight3.position){
        if (payload.rayDepth == 0) {
            //payload.directColor = materialBuffer.data[materialIndexBuffer.data[gl_PrimitiveID]].emission;
            payload.directColor = ivory.emission;
        }
        else {
            //payload.indirectColor += (1.0 / payload.rayDepth) * materialBuffer.data[materialIndexBuffer.data[gl_PrimitiveID]].emission * dot(payload.previousNormal, payload.rayDirection);
            payload.indirectColor += (1.0 / payload.rayDepth) * ivory.emission * dot(payload.previousNormal, payload.rayDirection);
        }
    } else {
        int randomIndex = int(random(gl_LaunchIDEXT.xy, camera.frameCount) * 2 + 40);
        vec3 lightColor = vec3(0.6, 0.6, 0.6);

        ivec3 lightIndices = ivec3(1,1,1);
        /*
        vec3 lightVertexA = vec3(rtxVertexBuffer.vertices[3 * lightIndices.x + 0], rtxVertexBuffer.vertices[3 * lightIndices.x + 1], rtxVertexBuffer.vertices[3 * lightIndices.x + 2]);
        vec3 lightVertexB = vec3(rtxVertexBuffer.vertices[3 * lightIndices.y + 0], rtxVertexBuffer.vertices[3 * lightIndices.y + 1], rtxVertexBuffer.vertices[3 * lightIndices.y + 2]);
        vec3 lightVertexC = vec3(rtxVertexBuffer.vertices[3 * lightIndices.z + 0], rtxVertexBuffer.vertices[3 * lightIndices.z + 1], rtxVertexBuffer.vertices[3 * lightIndices.z + 2]);
        */
        
        vec2 uv = vec2(random(gl_LaunchIDEXT.xy, camera.frameCount), random(gl_LaunchIDEXT.xy, camera.frameCount + 1));
        if (uv.x + uv.y > 1.0f) {
            uv.x = 1.0f - uv.x;
            uv.y = 1.0f - uv.y;
        }

        vec3 lightBarycentric = vec3(1.0 - uv.x - uv.y, uv.x, uv.y);
        //vec3 lightPosition = rtxLight.position.x * lightBarycentric.x + rtxLight.position.y * lightBarycentric.y + rtxLight.position.z * lightBarycentric.z;
        vec3 lightPosition;
        if(payload.rayOrigin == rtxLight1.position){
            lightPosition = vec3(dot(rtxLight1.position, lightBarycentric));
        }else if(payload.rayOrigin == rtxLight2.position){
            lightPosition = vec3(dot(rtxLight2.position, lightBarycentric));
        }else{
            lightPosition = vec3(dot(rtxLight3.position, lightBarycentric));
        }

        vec3 positionToLightDirection = normalize(lightPosition - position);

        vec3 shadowRayOrigin = position;
        vec3 shadowRayDirection = positionToLightDirection;
        float shadowRayDistance = length(lightPosition - position) - 0.001f;

        uint shadowRayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

        isShadow = true;
        traceRayEXT(tlas, shadowRayFlags, 0xFF, 0, 0, 1, shadowRayOrigin, 0.001, shadowRayDirection, shadowRayDistance, 1);

        // TODO: always true because light sources are dumb
        //if (!isShadow) {
            if (payload.rayDepth == 0) {
                payload.directColor = surfaceColor * lightColor * dot(geometricNormal, positionToLightDirection);
            }
            else {
                payload.indirectColor += (1.0 / payload.rayDepth) * surfaceColor * lightColor * dot(payload.previousNormal, payload.rayDirection) * dot(geometricNormal, positionToLightDirection);
            }
        //}
        //else {
        //    if (payload.rayDepth == 0) {
        //        payload.directColor = vec3(0.4, 0.4, 0.3); 
        //    }
        //    else {
        //        payload.rayActive = 0;
        //    }
        //}
    }

    vec3 hemisphere = uniformSampleHemisphere(vec2(random(gl_LaunchIDEXT.xy, camera.frameCount), random(gl_LaunchIDEXT.xy, camera.frameCount + 1)));
    vec3 alignedHemisphere = alignHemisphereWithCoordinateSystem(hemisphere, geometricNormal);

    payload.rayOrigin = position;
    payload.rayDirection = alignedHemisphere;
    payload.previousNormal = geometricNormal;

    payload.rayDepth += 1;
    
    //payload.directColor=vec3(1,0,0);
}
