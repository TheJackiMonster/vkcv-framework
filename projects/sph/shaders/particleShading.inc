float circleFactor(vec2 triangleCoordinates){
    // percentage of distance from center to circle edge
    float p = clamp((0.4 - length(triangleCoordinates)) / 0.4, 0, 1);
    // remapping for nice falloff
    return sqrt(p);
}