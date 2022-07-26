#ifndef VOXEL_INC
#define VOXEL_INC

#define VOXEL_NORM_VALUE 0xFF

#define voxel_add(img, pos, value) imageAtomicAdd(img, ivec3((imageSize(img) - ivec3(1)) * pos), uint(VOXEL_NORM_VALUE * value))

#define voxel_write(img, pos, value) imageStore(img, pos, uvec4(VOXEL_NORM_VALUE * value));
#define voxel_read(img, pos) imageLoad(img, pos).r / float(VOXEL_NORM_VALUE);

// https://stackoverflow.com/questions/51108596/linearize-depth
float linearize_depth(float d,float zNear,float zFar) {
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

#define voxel_pos(pos) vec3((pos.xy + vec2(1.0f)) * 0.5f, linearize_depth(pos.y, 0.1f, 50.0f))

#endif // VOXEL_INC