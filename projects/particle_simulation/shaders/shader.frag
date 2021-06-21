#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) in vec3 passVelocity;

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform uColor {
	vec4 color;
} Color;

layout(set=0,binding=1) uniform uPosition{
	vec2 position;
} Position;

void main()
{
	vec2 mouse = vec2(Position.position.x, Position.position.y);
		outColor = vec4(1,0,0,0);
		//outColor = float(distance(gl_FragCoord.xy, mouse) < 100) * vec4(0,0,1,0) +
		// 		   float(distance(gl_FragCoord.xy, mouse) >= 100) * Color.color;
}