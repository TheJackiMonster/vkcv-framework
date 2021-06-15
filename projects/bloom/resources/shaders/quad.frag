#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec3 outColor;

layout(set=0, binding=0) uniform texture2D  renderAttachment;
layout(set=0, binding=1) uniform texture2D  blurAttachment;
layout(set=0, binding=2) uniform sampler    samp;

void main()
{
    outColor = texture(sampler2D(renderAttachment, samp), inUV).rgb;
}