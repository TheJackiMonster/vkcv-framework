#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <chrono>
#include <vector>

namespace temp {

	struct Material {
		Material(const glm::vec3& e, const glm::vec3& color) : emission(e), albedo(color) {}

		glm::vec3   emission;
		float       padding0;
		glm::vec3   albedo;
		float       padding1;
	};

	struct Sphere {
		Sphere(const glm::vec3& c, const float& r, const int m) : center(c), radius(r), materialIndex(m) {}

		glm::vec3   center;
		float       radius;
		uint32_t    materialIndex;
		float       padding[3];
	};

	struct Plane {
		Plane(const glm::vec3& c, const glm::vec3& n, const glm::vec2 e, int m) 
			: center(c), normal(n), extent(e), materialIndex(m) {}

		glm::vec3   center;
		uint32_t    materialIndex;
		glm::vec3   normal;
		float       padding1;
		glm::vec2   extent;
		glm::vec2   padding3;
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
		true);

	vkcv::Core core = vkcv::Core::create(
		window,
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{ "VK_KHR_swapchain" }
	);

	// images
	vkcv::ImageHandle outputImage = core.createImage(
		vk::Format::eR32G32B32A32Sfloat,
		initialWidth,
		initialHeight,
		1,
		false,
		true).getHandle();

	vkcv::ImageHandle meanImage = core.createImage(
		vk::Format::eR32G32B32A32Sfloat,
		initialWidth,
		initialHeight,
		1,
		false,
		true).getHandle();

	vkcv::shader::GLSLCompiler compiler;

	// path tracing shader
	vkcv::ShaderProgram traceShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/path_tracer.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		traceShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& traceDescriptorBindings     = traceShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle traceDescriptorSetLayout    = core.createDescriptorSetLayout(traceDescriptorBindings);
	vkcv::DescriptorSetHandle       traceDescriptorSet          = core.createDescriptorSet(traceDescriptorSetLayout);

	// image combine shader
	vkcv::ShaderProgram imageCombineShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/combineImages.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		imageCombineShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& imageCombineDescriptorBindings  = imageCombineShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle imageCombineDescriptorSetLayout = core.createDescriptorSetLayout(imageCombineDescriptorBindings);
	vkcv::DescriptorSetHandle       imageCombineDescriptorSet       = core.createDescriptorSet(imageCombineDescriptorSetLayout);
	vkcv::PipelineHandle            imageCombinePipeline            = core.createComputePipeline(
		imageCombineShaderProgram, 
		{ core.getDescriptorSetLayout(imageCombineDescriptorSetLayout).vulkanHandle });

	vkcv::DescriptorWrites imageCombineDescriptorWrites;
	imageCombineDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(0, outputImage),
		vkcv::StorageImageDescriptorWrite(1, meanImage)
	};
	core.writeDescriptorSet(imageCombineDescriptorSet, imageCombineDescriptorWrites);

	// image present shader
	vkcv::ShaderProgram presentShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/presentImage.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		presentShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& presentDescriptorBindings   = presentShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle presentDescriptorSetLayout  = core.createDescriptorSetLayout(presentDescriptorBindings);
	vkcv::DescriptorSetHandle       presentDescriptorSet        = core.createDescriptorSet(presentDescriptorSetLayout);
	vkcv::PipelineHandle            presentPipeline             = core.createComputePipeline(
		presentShaderProgram,
		{ core.getDescriptorSetLayout(presentDescriptorSetLayout).vulkanHandle });

	// clear shader
	vkcv::ShaderProgram clearShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/clearImage.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		clearShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& imageClearDescriptorBindings    = clearShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle imageClearDescriptorSetLayout   = core.createDescriptorSetLayout(imageClearDescriptorBindings);
	vkcv::DescriptorSetHandle       imageClearDescriptorSet         = core.createDescriptorSet(imageClearDescriptorSetLayout);
	vkcv::PipelineHandle            imageClearPipeline              = core.createComputePipeline(
		clearShaderProgram,
		{ core.getDescriptorSetLayout(imageClearDescriptorSetLayout).vulkanHandle });

	vkcv::DescriptorWrites imageClearDescriptorWrites;
	imageClearDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(0, meanImage)
	};
	core.writeDescriptorSet(imageClearDescriptorSet, imageClearDescriptorWrites);

	// buffers
	std::vector<temp::Material> materials;
	materials.emplace_back(temp::Material(glm::vec3(0),     glm::vec3(0.65)));
	materials.emplace_back(temp::Material(glm::vec3(0),     glm::vec3(0.5, 0.0, 0.0)));
	materials.emplace_back(temp::Material(glm::vec3(0),     glm::vec3(0.0, 0.5, 0.0)));
    materials.emplace_back(temp::Material(glm::vec3(20),   glm::vec3(0)));

	const uint32_t whiteIndex   = 0;
	const uint32_t redIndex     = 1;
	const uint32_t greenIndex   = 2;
    const uint32_t lightIndex   = 3;

	std::vector<temp::Sphere> spheres;
	spheres.emplace_back(temp::Sphere(glm::vec3(0, -1.5, 0), 0.5, whiteIndex));

	std::vector<temp::Plane> planes;
	planes.emplace_back(temp::Plane(glm::vec3( 0, -2,    0), glm::vec3( 0,  1,  0), glm::vec2(2),   whiteIndex));
	planes.emplace_back(temp::Plane(glm::vec3( 0,  2,    0), glm::vec3( 0, -1,  0), glm::vec2(2),   whiteIndex));
	planes.emplace_back(temp::Plane(glm::vec3( 2,  0,    0), glm::vec3(-1,  0,  0), glm::vec2(2),   redIndex));
	planes.emplace_back(temp::Plane(glm::vec3(-2,  0,    0), glm::vec3( 1,  0,  0), glm::vec2(2),   greenIndex));
	planes.emplace_back(temp::Plane(glm::vec3( 0,  0,    2), glm::vec3( 0,  0, -1), glm::vec2(2),   whiteIndex));
	planes.emplace_back(temp::Plane(glm::vec3( 0,  1.9,  0), glm::vec3( 0, -1,  0), glm::vec2(1), lightIndex));

	vkcv::Buffer<temp::Sphere> sphereBuffer = core.createBuffer<temp::Sphere>(
		vkcv::BufferType::STORAGE,
		spheres.size());
	sphereBuffer.fill(spheres);

	vkcv::Buffer<temp::Plane> planeBuffer = core.createBuffer<temp::Plane>(
		vkcv::BufferType::STORAGE,
		planes.size());
	planeBuffer.fill(planes);

	vkcv::Buffer<temp::Material> materialBuffer = core.createBuffer<temp::Material>(
		vkcv::BufferType::STORAGE,
		materials.size());
	materialBuffer.fill(materials);

	vkcv::DescriptorWrites traceDescriptorWrites;
	traceDescriptorWrites.storageBufferWrites = { 
		vkcv::BufferDescriptorWrite(0, sphereBuffer.getHandle()),
		vkcv::BufferDescriptorWrite(1, planeBuffer.getHandle()),
		vkcv::BufferDescriptorWrite(2, materialBuffer.getHandle())};
	traceDescriptorWrites.storageImageWrites = {
		vkcv::StorageImageDescriptorWrite(3, outputImage)};
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
	int     frameIndex  = 0;
	bool    clearMeanImage = true;

	float cameraPitchPrevious   = 0;
	float cameraYawPrevious     = 0;
	glm::vec3 cameraPositionPrevious = glm::vec3(0);

	uint32_t widthPrevious  = initialWidth;
	uint32_t heightPrevious = initialHeight;

	while (window.isWindowOpen())
	{
		vkcv::Window::pollEvents();

		uint32_t swapchainWidth, swapchainHeight; // No resizing = No problem
		if (!core.beginFrame(swapchainWidth, swapchainHeight)) {
			continue;
		}

		if (swapchainWidth != widthPrevious || swapchainHeight != heightPrevious) {

			// resize images
			outputImage = core.createImage(
				vk::Format::eR32G32B32A32Sfloat,
				swapchainWidth,
				swapchainHeight,
				1,
				false,
				true).getHandle();

			meanImage = core.createImage(
				vk::Format::eR32G32B32A32Sfloat,
				swapchainWidth,
				swapchainHeight,
				1,
				false,
				true).getHandle();

			// update descriptor sets
			traceDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(3, outputImage) };
			core.writeDescriptorSet(traceDescriptorSet, traceDescriptorWrites);

			vkcv::DescriptorWrites imageCombineDescriptorWrites;
			imageCombineDescriptorWrites.storageImageWrites = {
				vkcv::StorageImageDescriptorWrite(0, outputImage),
				vkcv::StorageImageDescriptorWrite(1, meanImage)
			};
			core.writeDescriptorSet(imageCombineDescriptorSet, imageCombineDescriptorWrites);

			vkcv::DescriptorWrites imageClearDescriptorWrites;
			imageClearDescriptorWrites.storageImageWrites = {
				vkcv::StorageImageDescriptorWrite(0, meanImage)
			};
			core.writeDescriptorSet(imageClearDescriptorSet, imageClearDescriptorWrites);

			widthPrevious  = swapchainWidth;
			heightPrevious = swapchainHeight;

			clearMeanImage = true;
		}

		auto end = std::chrono::system_clock::now();
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - startTime);
		startTime = end;

		time += 0.000001f * static_cast<float>(deltatime.count());

		cameraManager.update(0.000001 * static_cast<double>(deltatime.count()));

		const vkcv::CommandStreamHandle cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		uint32_t fullscreenDispatchCount[3] = {
			static_cast<uint32_t> (std::ceil(swapchainWidth  / 8.f)),
			static_cast<uint32_t> (std::ceil(swapchainHeight / 8.f)),
			1 };

		float cameraPitch;
		float cameraYaw;
		cameraManager.getActiveCamera().getAngles(cameraPitch, cameraYaw);

		if (glm::abs(cameraPitch - cameraPitchPrevious) > 0.01 || glm::abs(cameraYaw - cameraYawPrevious) > 0.01)
			clearMeanImage = true;	// camera rotated

		cameraPitchPrevious = cameraPitch;
		cameraYawPrevious   = cameraYaw;

		glm::vec3 cameraPosition = cameraManager.getActiveCamera().getPosition();

		if(glm::distance(cameraPosition, cameraPositionPrevious) > 0.0001)
			clearMeanImage = true;	// camera moved

		cameraPositionPrevious = cameraPosition;

		if (clearMeanImage) {
			core.prepareImageForStorage(cmdStream, meanImage);

			core.recordComputeDispatchToCmdStream(cmdStream,
				imageClearPipeline,
				fullscreenDispatchCount,
				{ vkcv::DescriptorSetUsage(0, core.getDescriptorSet(imageClearDescriptorSet).vulkanHandle) },
				vkcv::PushConstants(0));

			clearMeanImage = false;
		}

		// path tracing
		struct RaytracingPushConstantData {
			glm::mat4   viewToWorld;
			int32_t     sphereCount;
			int32_t     planeCount;
			int32_t     frameIndex;
		};

		RaytracingPushConstantData raytracingPushData;
		raytracingPushData.viewToWorld = glm::inverse(cameraManager.getActiveCamera().getView());
		raytracingPushData.sphereCount = spheres.size();
		raytracingPushData.planeCount  = planes.size();
		raytracingPushData.frameIndex  = frameIndex;

		vkcv::PushConstants pushConstantsCompute(sizeof(RaytracingPushConstantData));
		pushConstantsCompute.appendDrawcall(raytracingPushData);

		uint32_t traceDispatchCount[3] = { 
			static_cast<uint32_t> (std::ceil(swapchainWidth  / 16.f)),
			static_cast<uint32_t> (std::ceil(swapchainHeight / 16.f)),
			1 };

		core.prepareImageForStorage(cmdStream, outputImage);

		core.recordComputeDispatchToCmdStream(cmdStream,
			tracePipeline,
			traceDispatchCount,
			{ vkcv::DescriptorSetUsage(0,core.getDescriptorSet(traceDescriptorSet).vulkanHandle) },
			pushConstantsCompute);

		core.prepareImageForStorage(cmdStream, meanImage);
		core.recordImageMemoryBarrier(cmdStream, outputImage);

		// combine images
		core.recordComputeDispatchToCmdStream(cmdStream,
			imageCombinePipeline,
			fullscreenDispatchCount,
			{ vkcv::DescriptorSetUsage(0,core.getDescriptorSet(imageCombineDescriptorSet).vulkanHandle) },
			vkcv::PushConstants(0));

		core.recordImageMemoryBarrier(cmdStream, meanImage);

		// present image
		const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

		vkcv::DescriptorWrites presentDescriptorWrites;
		presentDescriptorWrites.storageImageWrites = {
			vkcv::StorageImageDescriptorWrite(0, meanImage),
			vkcv::StorageImageDescriptorWrite(1, swapchainInput) };
		core.writeDescriptorSet(presentDescriptorSet, presentDescriptorWrites);

		core.prepareImageForStorage(cmdStream, swapchainInput);

		core.recordComputeDispatchToCmdStream(cmdStream,
			presentPipeline,
			fullscreenDispatchCount,
			{ vkcv::DescriptorSetUsage(0,core.getDescriptorSet(presentDescriptorSet).vulkanHandle) },
			vkcv::PushConstants(0));

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		core.endFrame();

		frameIndex++;
	}
	return 0;
}
