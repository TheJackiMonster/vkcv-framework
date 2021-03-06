#version 450

layout(location = 0) in vec2 passPos;
layout(location = 1) in float passMass;

layout(location = 0) out vec3 outColor;

void main()	{
    const float value = length(passPos);

    float z = sqrt(0.25 - value * value);

    if (value < 0.5f) {
        outColor = vec3(passPos.x + 0.5f, passPos.y + 0.5f, z * 2.0f);
    } else {
        discard;
    }
}