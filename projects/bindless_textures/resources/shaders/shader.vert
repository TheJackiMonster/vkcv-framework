#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 passNormal;
layout(location = 1) out vec2 passUV;
layout(location = 2) out flat int passTextureIndex;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()
{
	gl_Position = mvp * vec4(inPosition, 1.0);
	passNormal  = inNormal;
    passUV      = inUV;

    passTextureIndex = (gl_VertexIndex / 4);

    /*
    if(inNormal.x > 0.9)
        passTextureIndex = 0;

    if(inNormal.x < -0.9)
        passTextureIndex = 1;

    if(inNormal.y > 0.9)
        passTextureIndex = 2;

    if(inNormal.y < -0.9)
        passTextureIndex = 3;

    if(inNormal.z > 0.9)
        passTextureIndex = 4;

    if(inNormal.z < -0.9)
        passTextureIndex = 5;
    */
}