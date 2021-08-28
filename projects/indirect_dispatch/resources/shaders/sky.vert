#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout( push_constant ) uniform constants{
    mat4 viewProjection;
};

void main()	{
	gl_Position     = viewProjection * vec4(inPosition, 0.0);
    gl_Position.w   = gl_Position.z;
}