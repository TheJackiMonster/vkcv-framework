#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "voxel.inc"

layout(location = 0) out float passCubeHalf;
layout(location = 1) out vec3 passColorToGeom;

layout( push_constant ) uniform constants{
    mat4 viewProjection;
};

layout(set=0, binding=0, rgba16f) uniform image3D  voxelImage;
layout(set=0, binding=1) uniform voxelizationInfo{
    VoxelInfo voxelInfo;
};


void main()	{
    passCubeHalf        = voxelInfo.extent / float(imageSize(voxelImage).x) * 0.5f;
    int voxelResolution = imageSize(voxelImage).x;
    int slicePixelCount = voxelResolution * voxelResolution;
    int z               = gl_VertexIndex / slicePixelCount;
    int index2D         = gl_VertexIndex % slicePixelCount;
    int y               = index2D / voxelResolution;
    int x               = index2D % voxelResolution;
    vec3 position       = voxelCoordinatesToWorldPosition(ivec3(x, y, z), voxelResolution, voxelInfo, passCubeHalf);
	gl_Position         = vec4(position, 1.0);
    
    vec4 voxelColor = imageLoad(voxelImage, ivec3(x,y,z));
    if(voxelColor.a == 0){
        gl_Position.x /= 0; // clip
    }
    passColorToGeom = voxelColor.rgb;
}