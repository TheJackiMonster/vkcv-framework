#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 passNormal;
layout(location = 1) in vec2 passUV;

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform texture2D  meshTexture;
layout(set=0, binding=1) uniform sampler    textureSampler;

void main()	{
	vec3 lightDirection = normalize(vec3(0.1f, -0.9f, 0.1f));

	float ambient = 0.35f;
	float diffuse = max(0.0f, -dot(passNormal, lightDirection));
	float specular = pow(diffuse, 6.0f);

	float brightness = sqrt(
		(ambient + diffuse + specular) /
		(2.0f + ambient)
	);

	vec4 color = texture(sampler2D(meshTexture, textureSampler), passUV);

	if (color.a <= 0.0f) {
		discard;
	}

	outColor = vec4(color.rgb * brightness, color.a);
}