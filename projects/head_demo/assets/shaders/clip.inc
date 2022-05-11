
#define CLIP_SCALE 10000.0f

vec4 clipPosition(vec4 pos) {
    return vec4(
        max(clipX, pos.x),
        max(clipY, pos.y),
        max(clipZ, pos.z),
        1.0f / CLIP_SCALE
    );
}

vec4 clipByLimit(vec4 pos) {
    if (pos.x / pos.w < clipLimit) {
        return vec4(pos.xyz / pos.w, 1.0f);
    } else {
        return vec4(clipLimit, pos.y / pos.w, pos.z / pos.w, 1.0f);
    }
}
