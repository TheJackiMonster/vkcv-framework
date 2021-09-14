vec2 computeMotionVector(vec4 NDC, vec4 NDCPrevious){
    vec2 ndc            = NDC.xy            / NDC.w;
    vec2 ndcPrevious    = NDCPrevious.xy    / NDCPrevious.w;

    vec2 uv         = ndc           * 0.5 + 0.5;
    vec2 uvPrevious = ndcPrevious   * 0.5 + 0.5;

	return uvPrevious - uv;
}