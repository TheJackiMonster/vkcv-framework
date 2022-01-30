#version 450

layout(location = 0) in vec2 passPos;
layout(location = 1) in vec3 passVelocity;
layout(location = 2) in float passMass;

layout(location = 0) out vec3 outColor;

void main()	{
    if (passMass <= 0.0f) {
        discard;
    }

    const float value = length(passPos);

    float z = sqrt(0.25 - value * value);

    if (value < 0.5f) {
        vec3 surface = vec3(passPos.x + 0.5f, passPos.y + 0.5f, z * 2.0f);
        vec3 velocity = vec3(0.5f) + 0.5f * normalize(passVelocity.xyz);

        outColor = surface;
    } else {
        discard;
    }
}