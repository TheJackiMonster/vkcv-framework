#ifndef VOXEL_INC
#define VOXEL_INC

#define voxel_add(img, pos, value) imageAtomicAdd(img, ivec3((imageSize(img) - ivec3(1)) * pos), uint(0xFF * value))

#define voxel_write(img, pos, value) imageStore(img, pos, uvec4(0xFF * value));
#define voxel_read(img, pos) imageLoad(img, pos).r / 255.0f;

#endif // VOXEL_INC