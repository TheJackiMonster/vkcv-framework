#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

layout( push_constant ) uniform constants{
    mat4 mvp;
};

void main()	{
    vec3 positions[3] = {
        vec3(-1, -1, -1),
        vec3( 3, -1, -1),
        vec3(-1, 3, -1),
    };
    
    vec3 colors[3] = {
        vec3(1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, 0, 1)
    };

	gl_Position = mvp * vec4(positions[gl_VertexIndex], 1.0);
	fragColor = colors[gl_VertexIndex];
}