#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 outColor;

layout( push_constant ) uniform constants{
    vec3 skyColor;
    float skyStrength;
};

void main()	{
    outColor = skyColor * skyStrength;
}