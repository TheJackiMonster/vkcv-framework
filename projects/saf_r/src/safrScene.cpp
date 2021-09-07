#include "safrScene.hpp"


glm::vec3 safrScene::reflect(const glm::vec3& dir, const glm::vec3& hit_center) {
	return dir - hit_center * 2.f * (glm::dot(dir, hit_center));
}

bool safrScene::sceneIntersect(const glm::vec3& orig, const glm::vec3& dir, const std::vector<safrScene::Sphere>& spheres,
	glm::vec3& hit, glm::vec3& hit_center, safrScene::Material& material) {
	float spheres_dist = std::numeric_limits<float>::max();
	for (size_t i = 0; i < spheres.size(); i++) {
		float dist_i;
		if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
			spheres_dist = dist_i;
			hit = orig + dir * dist_i;
			hit_center = glm::normalize(hit - spheres[i].center);
			material = spheres[i].material;
		}
	}
	return spheres_dist < 1000;
}

glm::vec3 safrScene::castRay(const glm::vec3& orig, const glm::vec3& dir, const std::vector<safrScene::Sphere>& spheres,
	const std::vector<safrScene::Light>& lights, size_t depth = 0) {
	glm::vec3 point, hit_center;
	safrScene::Material material;

	//return background color if a max recursive depth is reached
	if (depth > 4 || !sceneIntersect(orig, dir, spheres, point, hit_center, material)) {
		return glm::vec3(0.2, 0.7, 0.8);
	}

	//compute recursive directions and origins of rays and then call the function
	glm::vec3 reflect_dir = glm::normalize(reflect(dir, hit_center));
	glm::vec3 reflect_orig = (glm::dot(reflect_dir, hit_center) < 0) ? point - hit_center * static_cast<float>(1e-3) :
		point + hit_center * static_cast<float>(1e-3); // offset the original point to avoid occlusion by the object itself
	glm::vec3 reflect_color = castRay(reflect_orig, reflect_dir, spheres, lights, depth + 1);

	//compute shadows and other light properties for the returned ray color
	float diffuse_light_intensity = 0, specular_light_intensity = 0;
	for (size_t i = 0; i < lights.size(); i++) {
		glm::vec3 light_dir = glm::normalize(lights[i].position - point);
		float light_distance = glm::distance(lights[i].position, point);

		glm::vec3 shadow_orig = (glm::dot(light_dir, hit_center) < 0) ? point - hit_center * static_cast<float>(1e-3) :
			point + hit_center * static_cast<float>(1e-3); // checking if the point lies in the shadow of the lights[i]
		glm::vec3 shadow_pt, shadow_hit_center;
		safrScene::Material tmpmaterial;
		if (sceneIntersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_hit_center, tmpmaterial)
			&& glm::distance(shadow_pt, shadow_orig) < light_distance)
			continue;
		diffuse_light_intensity += lights[i].intensity * std::max(0.f, glm::dot(light_dir, hit_center));
		specular_light_intensity += powf(std::max(0.f, glm::dot(reflect(light_dir, hit_center), dir)), material.specular_exponent) * lights[i].intensity;
	}
	return material.diffuse_color * diffuse_light_intensity * material.albedo[0] +
		glm::vec3(1., 1., 1.) * specular_light_intensity * material.albedo[1] + reflect_color * material.albedo[2];
}

vkcv::asset::Texture safrScene::render(const std::vector<safrScene::Sphere>& spheres, const std::vector<safrScene::Light>& lights) {
	//constants for the image data
	const int width = 800;
	const int height = 600;
	const int fov = M_PI / 2.;

	//compute image format for the framebuffer and compute the ray colors for the image
	std::vector<glm::vec3> framebuffer(width * height);
#pragma omp parallel for
	for (size_t j = 0; j < height; j++) {
		for (size_t i = 0; i < width; i++) {
			//framebuffer[i + j * width] = glm::vec3(j / float(height), i / float(width), 0);
			float x = (2 * (i + 0.5f) / (float)width - 1) * tan(fov / 2.f) * width / (float)height;
			float y = -(2 * (j + 0.5f) / (float)height - 1) * tan(fov / 2.f);
			glm::vec3 dir = glm::normalize(glm::vec3(x, y, -1));
			framebuffer[i + j * width] = castRay(glm::vec3(0, 0, 0), dir, spheres, lights);
		}
	}

	std::vector<uint8_t> data;
	for (size_t i = 0; i < height * width; ++i) {
		glm::vec3& c = framebuffer[i];
		float max = std::max(c[0], std::max(c[1], c[2]));
		if (max > 1) c = c * (1.f / max);
		data.push_back(static_cast<uint8_t>(255.f * framebuffer[i].x));
		data.push_back(static_cast<uint8_t>(255.f * framebuffer[i].y));
		data.push_back(static_cast<uint8_t>(255.f * framebuffer[i].z));
		data.push_back(static_cast<uint8_t>(255.f));
	}

	vkcv::asset::Texture textureData;

	textureData.width = width;
	textureData.height = height;
	textureData.channels = 4;

	textureData.data.resize(textureData.width * textureData.height * textureData.channels);
	memcpy(textureData.data.data(), data.data(), textureData.data.size());
	return textureData;
}