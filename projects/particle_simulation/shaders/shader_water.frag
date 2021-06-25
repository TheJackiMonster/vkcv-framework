#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 1) in vec3 passVelocity;
layout(location = 2) in float passlifeTime;

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform uColor {
	vec4 color;
} Color;

layout(set=0,binding=1) uniform uPosition{
	vec2 position;
} Position;

void main()
{
	float normlt = 1-normalize(passlifeTime);
	vec2 mouse = vec2(Position.position.x, Position.position.y);
	outColor =float(passlifeTime < 1) * vec4(0.2,0.5,1,0) +
	float(passlifeTime < 2 && passlifeTime > 1) * vec4(0.3, 0.7,1,0) +
	float(passlifeTime >= 2 && passlifeTime < 4.f) * vec4(0.5,0.9,1,0) +
	float(passlifeTime >= 4.f) * vec4(0.9,1,1,0);
}
