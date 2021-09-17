#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;
layout(location = 2) out vec3 passPos;
layout(location = 3) out vec4 passTangent;

layout( push_constant ) uniform constants{
    mat4 mvp;
    mat4 model;
};

void main()	{
	gl_Position = mvp * vec4(inPosition, 1.0);
	passNormal  = mat3(model) * inNormal;    // assuming no weird stuff like shearing or non-uniform scaling
    passUV      = inUV;
    passPos     = (model * vec4(inPosition, 1)).xyz;
    passTangent = vec4(mat3(model) * inTangent.xyz, inTangent.w);
}