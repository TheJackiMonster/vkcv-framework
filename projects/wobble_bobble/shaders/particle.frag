#version 450

layout(location = 0) in vec2 passPos;

layout(location = 0) out vec4 outColor;

void main()	{
    const float value = length(passPos);

    if (value < 0.5f) {
        outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f - value * 2.0f);
    } else {
        discard;
    }
}