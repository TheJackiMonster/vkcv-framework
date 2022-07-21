#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "physics.inc"

layout(location = 0) in vec3 passPos;
layout(location = 1) in vec3 passView;
layout(location = 2) in vec3 passColor;
layout(location = 3) in float passDensity;
layout(location = 4) in flat int passSmokeIndex;

layout(location = 0) out vec4 outColor;

layout(set=1, binding=0, std430) readonly buffer randomBuffer {
    float randomData [];
};

layout( push_constant ) uniform constants{
    mat4 view;
    mat4 projection;
};

#define NUM_SMOKE_SAMPLES 16

void main()	{
    if (passDensity <= mediumDensity) {
        discard;
    }

    vec3 dir = -normalize((inverse(view) * vec4(passView, 0)).xyz);

    vec3 start = passPos;
    vec3 end = start + dir * 3.5f;

    vec4 result = vec4(0);

    for (uint i = 0; i < NUM_SMOKE_SAMPLES; i++) {
        vec3 position = (
            end + (start - end) * i / (NUM_SMOKE_SAMPLES - 1)
        );

        vec4 data = vec4(passColor, passDensity);

        float fallOff = max(1.0f - length(position), 0.0f);

        const uint randomIndex = (passSmokeIndex * NUM_SMOKE_SAMPLES + i) % randomData.length();
        const float alpha = (1.0f + randomData[randomIndex] * 0.1f) * data.a * fallOff;

        result = vec4(
            (result.rgb + data.rgb * alpha) * (1.0f - result.a),
            result.a + (alpha) * (1.0f - result.a)
        );
    }

    result.r = clamp(result.r, 0, 1);
    result.g = clamp(result.g, 0, 1);
    result.b = clamp(result.b, 0, 1);

    // Inverse alpha value
    result.a = 1.0f - clamp(result.a, 0, 1);;

    if (result.a < 1.0f) {
        outColor = result;
    } else {
        discard;
    }
}