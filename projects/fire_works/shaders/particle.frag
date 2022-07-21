#version 450

layout(location = 0) in vec2 passPos;
layout(location = 1) in flat vec3 passColor;
layout(location = 2) in flat float passLifetime;

layout(location = 0) out vec4 outColor;

void main()	{
    if (passLifetime <= 0.0f) {
        discard;
    }

    const float value = length(passPos);

    if (value < 0.5f) {
        outColor = vec4(passColor, 1.0f - max(value * 2.0f, 0.0f));
    } else {
        discard;
    }
}