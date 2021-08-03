#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 passNDC;
layout(location = 1) in vec4 passNDCPrevious;

layout(location = 0) out vec2 outMotion;

void main()	{

    vec2 ndc            = passNDC.xy            / passNDC.w;
    vec2 ndcPrevious    = passNDCPrevious.xy    / passNDCPrevious.w;

    vec2 uv         = ndc           * 0.5 + 0.5;
    vec2 uvPrevious = ndcPrevious   * 0.5 + 0.5;

	outMotion = uvPrevious - uv;
}