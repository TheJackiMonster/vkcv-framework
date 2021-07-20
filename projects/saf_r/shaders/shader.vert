#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
    vec3 positions[3] = {
        vec3(-1, -1, -1),
        vec3( 3, -1, -1),
        vec3(-1, 3, -1)
    };
    
    vec3 colors[3] = {
        vec3(1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, 0, 1)
    };

    vec4 position = mvp * vec4(positions[gl_VertexIndex], 1.0);
	gl_Position = position;

    texCoord.x = (position.x + 1.0) * 0.5;
    texCoord.y = (position.y + 1.0) * 0.5;

	fragColor = colors[gl_VertexIndex];
}