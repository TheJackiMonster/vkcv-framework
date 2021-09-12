#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <vector>

namespace temp {

	struct Light {
		Light(const glm::vec3& p, const float& i) : position(p), intensity(i) {}
		glm::vec3 position;
		float intensity;
	};

	struct Material {
		Material(const glm::vec3& a, const glm::vec3& color, const float& spec) : albedo(a), diffuse_color(color), specular_exponent(spec) {}
		Material() : albedo(1, 0, 0), diffuse_color(), specular_exponent() {}
		glm::vec3 albedo;
		alignas(16) glm::vec3 diffuse_color;
		float specular_exponent;
	};

	struct Sphere {
		glm::vec3 center;
		float radius;
		Material material;

		Sphere(const glm::vec3& c, const float& r, const Material& m) : center(c), radius(r), material(m) {}
	};
};

int main(int argc, const char** argv) {
	const char* applicationName = "Path Tracer";

	const int initialWidth = 1280;
	const int initialHeight = 720;
	vkcv::Window window = vkcv::Window::create(
		applicationName,
		initialWidth,
		initialHeight,
		false);

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{ "VK_KHR_swapchain" }
	);

	vkcv::shader::GLSLCompiler compiler;
	vkcv::ShaderProgram traceShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/path_tracer.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		traceShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& traceDescriptorBindings     = traceShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle traceDescriptorSetLayout    = core.createDescriptorSetLayout(traceDescriptorBindings);
	vkcv::DescriptorSetHandle       traceDescriptorSet          = core.createDescriptorSet(traceDescriptorSetLayout);

	std::vector<temp::Material> materials;
	temp::Material ivory(glm::vec3(0.6, 0.3, 0.1), glm::vec3(0.4, 0.4, 0.3), 50.);
	temp::Material red_rubber(glm::vec3(0.9, 0.1, 0.0), glm::vec3(0.3, 0.1, 0.1), 10.);
	temp::Material mirror(glm::vec3(0.0, 10.0, 0.8), glm::vec3(1.0, 1.0, 1.0), 1425.);
	materials.push_back(ivory);
	materials.push_back(red_rubber);
	materials.push_back(mirror);

	std::vector<temp::Sphere> spheres;
	spheres.push_back(temp::Sphere(glm::vec3(-3.0, 0.0, 16), 2, ivory));
	spheres.push_back(temp::Sphere(glm::vec3(-1.0, -1.5, 12), 2, mirror));
	spheres.push_back(temp::Sphere(glm::vec3(1.5, -0.5, 18), 3, red_rubber));
	spheres.push_back(temp::Sphere(glm::vec3(7.0, 5.0, 18), 4, mirror));

	std::vector<temp::Light> lights;
	lights.push_back(temp::Light(glm::vec3(-20, 20, 20), 1.5));
	lights.push_back(temp::Light(glm::vec3(30, 50, -25), 1.8));
	lights.push_back(temp::Light(glm::vec3(30, 20, 30), 1.7));

	vkcv::Buffer<temp::Light> lightsBuffer = core.createBuffer<temp::Light>(
		vkcv::BufferType::STORAGE,
		lights.size()
		);
	lightsBuffer.fill(lights);

	vkcv::Buffer<temp::Sphere> sphereBuffer = core.createBuffer<temp::Sphere>(
		vkcv::BufferType::STORAGE,
		spheres.size()
		);
	sphereBuffer.fill(spheres);

	vkcv::DescriptorWrites traceDescriptorWrites;
	traceDescriptorWrites.storageBufferWrites = { 
		vkcv::BufferDescriptorWrite(0,lightsBuffer.getHandle()),
		vkcv::BufferDescriptorWrite(1,sphereBuffer.getHandle()) };
	core.writeDescriptorSet(traceDescriptorSet, traceDescriptorWrites);

	vkcv::PipelineHandle tracePipeline = core.createComputePipeline(
		traceShaderProgram, 
		{ core.getDescriptorSetLayout(traceDescriptorSetLayout).vulkanHandle });

	if (!tracePipeline)
	{
		vkcv_log(vkcv::LogLevel::ERROR, "Could not create graphics pipeline. Exiting.");
		return EXIT_FAILURE;
	}

	vkcv::camera::CameraManager cameraManager(window);
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);

	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));

	auto    startTime   = std::chrono::system_clock::now();
	float   time        = 0;

	while (window.isWindowOpen())
	{
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime);
		startTime = end;

		time += 0.000001f * static_cast<float>(deltatime.count());

		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		const vkcv::CommandStreamHandle cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
		traceDescriptorWrites.storageImageWrites = { vkcv::StorageImageDescriptorWrite(2, swapchainInput) };
		core.writeDescriptorSet(traceDescriptorSet, traceDescriptorWrites);

		core.prepareImageForStorage(cmdStream, swapchainInput);

		struct RaytracingPushConstantData {
			glm::mat4   viewToWorld;
			int32_t     lightCount;
			int32_t     sphereCount;
		};

		RaytracingPushConstantData raytracingPushData;
		raytracingPushData.lightCount  = lights.size();
		raytracingPushData.sphereCount = spheres.size();
		raytracingPushData.viewToWorld = glm::inverse(cameraManager.getActiveCamera().getView());

		vkcv::PushConstants pushConstantsCompute(sizeof(RaytracingPushConstantData));
		pushConstantsCompute.appendDrawcall(raytracingPushData);

		uint32_t computeDispatchCount[3] = { 
			static_cast<uint32_t> (std::ceil(swapchainWidth  / 16.f)),
			static_cast<uint32_t> (std::ceil(swapchainHeight / 16.f)),
			1 };

		core.recordComputeDispatchToCmdStream(cmdStream,
			tracePipeline,
			computeDispatchCount,
			{ vkcv::DescriptorSetUsage(0,core.getDescriptorSet(traceDescriptorSet).vulkanHandle) },
			pushConstantsCompute);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame();
	}
	return 0;
}
