#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 passPos;
layout(location = 1) in flat vec3 passColor;
layout(location = 2) in flat float passSize;
layout(location = 3) in flat int passTextureIndex;

layout(location = 0) out vec4 outColor;

layout(set=1, binding=0) uniform sampler smokeSampler;
layout(set=1, binding=1) uniform texture3D smokeTextures [];

#define NUM_SMOKE_SAMPLES 8

void main()	{
    if (passSize <= 0.0f) {
        discard;
    }

    vec3 start = (vec3(1.0f) + passPos) * 0.5f;
    vec3 end = (vec3(1.0f) - passPos) * 0.5f;

    vec4 result = vec4(0);

    for (uint i = 0; i < NUM_SMOKE_SAMPLES; i++) {
#if NUM_SMOKE_SAMPLES > 1
        vec3 position = mix(end, start, vec3(i) / (NUM_SMOKE_SAMPLES - 1));
#else
        vec3 position = end;
#endif

        vec4 data = texture(
            sampler3D(
                smokeTextures[nonuniformEXT(passTextureIndex)],
                smokeSampler
            ),
            position
        );

        result = vec4(mix(result.rgb, data.rgb, data.a), min(result.a + data.a, 1.0f));
    }

    outColor = vec4(passColor * result.rgb, 0.1f * result.a);
}