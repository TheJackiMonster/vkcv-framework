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
		Material(const glm::vec3& a, const glm::vec3& color, const float& spec) : albedo(a), diffuse_color(color), specular_exponent(spec) {}
		Material() : albedo(1, 0, 0), diffuse_color(), specular_exponent() {}
		glm::vec3 albedo;
		alignas(16) glm::vec3 diffuse_color;
		float specular_exponent;
	};

	/*
	* the sphere is defined by it's center, the radius and the material
	*
	* the ray_intersect function checks, if a ray from the raytracer passes through the sphere, hits the sphere or passes by the the sphere
	* @param vec3: origin of ray
	* @param vec3: direction of ray
	* @param float: distance of the ray to the sphere
	* @return bool: if ray interesects sphere or not
	*/
	struct Sphere {
		glm::vec3 center;
		float radius;
		Material material;

		Sphere(const glm::vec3& c, const float& r, const Material& m) : center(c), radius(r), material(m) {}

		bool ray_intersect(const glm::vec3& origin, const glm::vec3& dir, float& t0) const {
			glm::vec3 L = center - origin;
			float tca = glm::dot(L, dir);
			float d2 = glm::dot(L, L) - tca * tca;
			if (d2 > radius * radius) return false;
			float thc = sqrtf(radius * radius - d2);
			t0 = tca - thc;
			float t1 = tca + thc;
			if (t0 < 0) t0 = t1;
			if (t0 < 0) return false;
			return true;
		}
	};

	/*
	* @param vector: all spheres in the scene
	* @param vector: all light sources in the scene
	* @return TextureData: texture data for the buffers
	*/
	vkcv::asset::Texture render(const std::vector<Sphere>& spheres, const std::vector<Light>& lights);

private:
	/*
	* @param vec3 dir: direction of the ray
	* @param vec3 hit_center: normalized vector between hit on the sphere and center of the sphere
	* @return vec3: returns reflected vector for the new direction of the ray
	*/
	glm::vec3 reflect(const glm::vec3& dir, const glm::vec3& hit_center);

	/*
	* @param orig: Origin of the ray
	* @param dir: direction of the ray
	* @param vector: vector of all spheres in the scene
	* @param vec3 hit: returns the vector from the origin of the ray to the closest sphere
	* @param vec3 N: normalizes the vector from the origin of the ray to the closest sphere center
	* @param Material: returns the material of the closest sphere
	* @return: closest sphere distance if it's < 1000
	*/
	bool sceneIntersect(const glm::vec3& orig, const glm::vec3& dir, const std::vector<Sphere>& spheres,
		glm::vec3& hit, glm::vec3& hit_center, Material& material);

	/*
	* @param vec3 orig: origin of the ray
	* @param vec3 dir: direction of the ray
	* @param vector: all spheres in the scene
	* @param vector: all light sources of the scene
	* @param depth = 0: initial recrusive depth
	* @return color of the pixel depending on material and light
	*/
	glm::vec3 castRay(const glm::vec3& orig, const glm::vec3& dir, const std::vector<Sphere>& spheres,
		const std::vector<Light>& lights, size_t depth);
	
};