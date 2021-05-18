#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 fragColor;

void main()	{
    vec3 positions[3] = {
        vec3(-0.5, 0.5, 0),
        vec3( 0.5, 0.5, 0),
        vec3(0, -0.5, 0)
    };
    
    vec3 colors[3] = {
        vec3(1, 0, 0),
        vec3(0, 1, 0),
        vec3(0, 0, 1)
    };

	gl_Position = vec4(positions[gl_VertexIndex], 1.0);
	fragColor = colors[gl_VertexIndex];
}