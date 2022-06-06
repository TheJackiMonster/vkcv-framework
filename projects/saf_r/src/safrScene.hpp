#include <iostream>
#include <vkcv/Core.hpp>
#include <GLFW/glfw3.h>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <limits>
#include <cmath>
#include <vector>
#include <string.h>	// memcpy(3)

class safrScene {

public:

	/*
	* Light struct with a position and intensity of the light source
	*/
	struct Light {
		Light(const glm::vec3& p, const float& i) : position(p), intensity(i) {}
		glm::vec3 position;
		float intensity;
	};

	/*
	* Material struct with defuse color, albedo and specular component
	*/
	struct Material {
		Material(const glm::vec4& a, const glm::vec3& color, const float& spec, const float& r) : albedo(a), diffuse_color(color), specular_exponent(spec), refractive_index(r) {}
		Material() : albedo(1, 0, 0, 0), diffuse_color(), specular_exponent(), refractive_index(1) {}
        glm::vec4 albedo;
        alignas(16) glm::vec3 diffuse_color;
        float specular_exponent;
		float refractive_index;
	};

	/*
	* the sphere is defined by it's center, the radius and the material
	*/
	struct Sphere {
		glm::vec3 center;
		float radius;
		Material material;

		Sphere(const glm::vec3& c, const float& r, const Material& m) : center(c), radius(r), material(m) {}

	};

	
};