#ifndef VOXEL_INC
#define VOXEL_INC

#define VOXEL_NORM_VALUE 0xFF

#define voxel_add(img, pos, value) imageAtomicAdd(img, ivec3((imageSize(img) - ivec3(1)) * pos), uint(VOXEL_NORM_VALUE * value))

#define voxel_write(img, pos, value) imageStore(img, pos, uvec4(VOXEL_NORM_VALUE * value));
#define voxel_read(img, pos) imageLoad(img, pos).r / float(VOXEL_NORM_VALUE);

#endif // VOXEL_INC