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

void main()	{
    if (passSize <= 0.0f) {
        discard;
    }

    vec4 data = texture(
        sampler3D(
            smokeTextures[nonuniformEXT(passTextureIndex)],
            smokeSampler
        ),
        vec3(0.0f)
    );

    outColor = vec4(passColor + data.rgb, 0.1f + data.a);
}