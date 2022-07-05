#version 450

layout(location = 0) in vec2 passPos;
layout(location = 1) in vec3 passColor;

layout(location = 0) out vec3 outColor;

void main()	{
    const float value = length(passPos);

    if (value < 0.5f) {
        outColor = passColor;
    } else {
        discard;
    }
}