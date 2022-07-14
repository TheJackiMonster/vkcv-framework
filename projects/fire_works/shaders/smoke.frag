#version 450

layout(location = 0) in vec3 passPos;
layout(location = 1) in vec3 passColor;
layout(location = 2) in float passSize;

layout(location = 0) out vec4 outColor;

void main()	{
    if (passSize <= 0.0f) {
        discard;
    }

    outColor = vec4(passColor, 0.1f);
}