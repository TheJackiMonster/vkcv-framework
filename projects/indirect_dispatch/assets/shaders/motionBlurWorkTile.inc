#ifndef MOTION_BLUR_WORK_TILE
#define MOTION_BLUR_WORK_TILE

#include "motionBlurConfig.inc"

const int maxTileCount = 
    (maxMotionBlurWidth  + motionTileSize - 1) / motionTileSize * 
    (maxMotionBlurHeight + motionTileSize - 1) / motionTileSize;

struct WorkTiles{
    uint    tileCount;
    // dispatch Y/Z are here so the buffer can be used directly as an indirect dispatch argument buffer
    uint    dispatchY;
    uint    dispatchZ;
    
    ivec2   tileXY[maxTileCount];
};

#endif // #ifndef MOTION_BLUR_WORK_TILE