#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/asset/asset_loader.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "vkcv/gui/GUI.hpp"
#include <chrono>
#include <vector>

int main(int argc, const char** argv) {

	// structs must match shader version
	struct Material {
		Material(const glm::vec3& emission, const glm::vec3& albedo, float ks, float roughness, const glm::vec3& f0)
			: emission(emission), ks(ks), albedo(albedo), roughness(roughness), f0(f0), padding() {}

		glm::vec3   emission;
		float       ks;
		glm::vec3   albedo;
		float       roughness;
		glm::vec3   f0;
		float       padding;
	};

	struct Sphere {
		Sphere(const glm::vec3& c, const float& r, const int m)
			: center(c), radius(r), materialIndex(m), padding() {}

		glm::vec3   center;
		float       radius;
		uint32_t    materialIndex;
		float       padding[3];
	};

	struct Plane {
		Plane(const glm::vec3& c, const glm::vec3& n, const glm::vec2 e, int m)
			: center(c), materialIndex(m), normal(n), padding1(), extent(e), padding3() {}

		glm::vec3   center;
		uint32_t    materialIndex;
		glm::vec3   normal;
		float       padding1;
		glm::vec2   extent;
		glm::vec2   padding3;
	};

	const std::string applicationName = "Path Tracer";

	vkcv::Core core = vkcv::Core::create(
		applicationName,
		VK_MAKE_VERSION(0, 0, 1),
		{ vk::QueueFlagBits::eTransfer,vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute },
		{ "VK_KHR_swapchain" }
	);
	
	const int initialWidth = 1280;
	const int initialHeight = 720;
	
	vkcv::WindowHandle windowHandle = core.createWindow(
			applicationName,
			initialWidth,
			initialHeight,
			true
	);

	// images
	vkcv::ImageHandle outputImage = core.createImage(
		vk::Format::eR32G32B32A32Sfloat,
		initialWidth,
		initialHeight,
		1,
		false,
		true
	);

	vkcv::ImageHandle meanImage = core.createImage(
		vk::Format::eR32G32B32A32Sfloat,
		initialWidth,
		initialHeight,
		1,
		false,
		true
	);

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
	vkcv::ComputePipelineHandle     imageCombinePipeline            = core.createComputePipeline({
		imageCombineShaderProgram, 
		{ imageCombineDescriptorSetLayout }
	});

	vkcv::DescriptorWrites imageCombineDescriptorWrites;
	imageCombineDescriptorWrites.writeStorageImage(0, outputImage).writeStorageImage(1, meanImage);
	core.writeDescriptorSet(imageCombineDescriptorSet, imageCombineDescriptorWrites);

	// image present shader
	vkcv::ShaderProgram presentShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/presentImage.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		presentShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& presentDescriptorBindings   = presentShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle presentDescriptorSetLayout  = core.createDescriptorSetLayout(presentDescriptorBindings);
	vkcv::DescriptorSetHandle       presentDescriptorSet        = core.createDescriptorSet(presentDescriptorSetLayout);
	vkcv::ComputePipelineHandle     presentPipeline             = core.createComputePipeline({
		presentShaderProgram,
		{ presentDescriptorSetLayout }
	});

	// clear shader
	vkcv::ShaderProgram clearShaderProgram{};

	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/clearImage.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		clearShaderProgram.addShader(shaderStage, path);
	});

	const vkcv::DescriptorBindings& imageClearDescriptorBindings    = clearShaderProgram.getReflectedDescriptors().at(0);
	vkcv::DescriptorSetLayoutHandle imageClearDescriptorSetLayout   = core.createDescriptorSetLayout(imageClearDescriptorBindings);
	vkcv::DescriptorSetHandle       imageClearDescriptorSet         = core.createDescriptorSet(imageClearDescriptorSetLayout);
	vkcv::ComputePipelineHandle     imageClearPipeline              = core.createComputePipeline({
		clearShaderProgram,
		{ imageClearDescriptorSetLayout }
	});

	vkcv::DescriptorWrites imageClearDescriptorWrites;
	imageClearDescriptorWrites.writeStorageImage(0, meanImage);
	core.writeDescriptorSet(imageClearDescriptorSet, imageClearDescriptorWrites);

	// buffers
	typedef std::pair<std::string, Material> MaterialSetting;

	std::vector<MaterialSetting> materialSettings;
	materialSettings.emplace_back(MaterialSetting("white",  Material(glm::vec3(0),    glm::vec3(0.65),          0, 0.25, glm::vec3(0.04))));
	materialSettings.emplace_back(MaterialSetting("red",    Material(glm::vec3(0),    glm::vec3(0.5, 0.0, 0.0), 0, 0.25, glm::vec3(0.04))));
	materialSettings.emplace_back(MaterialSetting("green",  Material(glm::vec3(0),    glm::vec3(0.0, 0.5, 0.0), 0, 0.25, glm::vec3(0.04))));
	materialSettings.emplace_back(MaterialSetting("light",  Material(glm::vec3(20),   glm::vec3(0),             0, 0.25, glm::vec3(0.04))));
	materialSettings.emplace_back(MaterialSetting("sphere", Material(glm::vec3(0),    glm::vec3(0.65),          1, 0.25, glm::vec3(0.04))));
	materialSettings.emplace_back(MaterialSetting("ground", Material(glm::vec3(0),    glm::vec3(0.65),          0, 0.25, glm::vec3(0.04))));

	const uint32_t whiteMaterialIndex   = 0;
	const uint32_t redMaterialIndex     = 1;
	const uint32_t greenMaterialIndex   = 2;
	const uint32_t lightMaterialIndex   = 3;
	const uint32_t sphereMaterialIndex  = 4;
	const uint32_t groundMaterialIndex  = 5;

	std::vector<Sphere> spheres;
	spheres.emplace_back(Sphere(glm::vec3(0, -1.5, 0), 0.5, sphereMaterialIndex));

	std::vector<Plane> planes;
	planes.emplace_back(Plane(glm::vec3( 0, -2,     0), glm::vec3( 0,  1,  0), glm::vec2(2), groundMaterialIndex));
	planes.emplace_back(Plane(glm::vec3( 0,  2,     0), glm::vec3( 0, -1,  0), glm::vec2(2), whiteMaterialIndex));
	planes.emplace_back(Plane(glm::vec3( 2,  0,     0), glm::vec3(-1,  0,  0), glm::vec2(2), redMaterialIndex));
	planes.emplace_back(Plane(glm::vec3(-2,  0,     0), glm::vec3( 1,  0,  0), glm::vec2(2), greenMaterialIndex));
	planes.emplace_back(Plane(glm::vec3( 0,  0,     2), glm::vec3( 0,  0, -1), glm::vec2(2), whiteMaterialIndex));
	planes.emplace_back(Plane(glm::vec3( 0,  1.9,   0), glm::vec3( 0, -1,  0), glm::vec2(1), lightMaterialIndex));

	vkcv::Buffer<Sphere> sphereBuffer = vkcv::buffer<Sphere>(
		core,
		vkcv::BufferType::STORAGE,
		spheres.size());
	sphereBuffer.fill(spheres);

	vkcv::Buffer<Plane> planeBuffer = vkcv::buffer<Plane>(
		core,
		vkcv::BufferType::STORAGE,
		planes.size());
	planeBuffer.fill(planes);

	vkcv::Buffer<Material> materialBuffer = vkcv::buffer<Material>(
		core,
		vkcv::BufferType::STORAGE,
		materialSettings.size());

	vkcv::DescriptorWrites traceDescriptorWrites;
	traceDescriptorWrites.writeStorageBuffer(
			0, sphereBuffer.getHandle()
	).writeStorageBuffer(
			1, planeBuffer.getHandle()
	).writeStorageBuffer(
			2, materialBuffer.getHandle()
	);
	
	traceDescriptorWrites.writeStorageImage(3, outputImage);
	core.writeDescriptorSet(traceDescriptorSet, traceDescriptorWrites);

	vkcv::ComputePipelineHandle tracePipeline = core.createComputePipeline({
		traceShaderProgram,
		{ traceDescriptorSetLayout }
	});

	if (!tracePipeline)
	{
		vkcv_log(vkcv::LogLevel::ERROR, "Could not create graphics pipeline. Exiting.");
		return EXIT_FAILURE;
	}

	vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
	uint32_t camIndex0 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);

	cameraManager.getCamera(camIndex0).setPosition(glm::vec3(0, 0, -2));
	
	int     frameIndex      = 0;
	bool    clearMeanImage  = true;
	bool    updateMaterials = true;

	float cameraPitchPrevious   = 0;
	float cameraYawPrevious     = 0;
	glm::vec3 cameraPositionPrevious = glm::vec3(0);

	uint32_t widthPrevious  = initialWidth;
	uint32_t heightPrevious = initialHeight;

	vkcv::gui::GUI gui(core, windowHandle);

	bool renderUI = true;
	core.getWindow(windowHandle).e_key.add([&renderUI](int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_I && action == GLFW_PRESS) {
			renderUI = !renderUI;
		}
	});

	glm::vec3   skyColor            = glm::vec3(0.2, 0.7, 0.8);
	float       skyColorMultiplier  = 1;
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if (swapchainWidth != widthPrevious || swapchainHeight != heightPrevious) {

			// resize images
			outputImage = core.createImage(
				vk::Format::eR32G32B32A32Sfloat,
				swapchainWidth,
				swapchainHeight,
				1,
				false,
				true
			);

			meanImage = core.createImage(
				vk::Format::eR32G32B32A32Sfloat,
				swapchainWidth,
				swapchainHeight,
				1,
				false,
				true
			);

			// update descriptor sets
			traceDescriptorWrites.writeStorageImage(3, outputImage);
			core.writeDescriptorSet(traceDescriptorSet, traceDescriptorWrites);

			vkcv::DescriptorWrites imageCombineDescriptorWrites;
			imageCombineDescriptorWrites.writeStorageImage(
					0, outputImage
			).writeStorageImage(
					1, meanImage
			);
			
			core.writeDescriptorSet(imageCombineDescriptorSet, imageCombineDescriptorWrites);

			vkcv::DescriptorWrites imageClearDescriptorWrites;
			imageClearDescriptorWrites.writeStorageImage(0, meanImage);
			core.writeDescriptorSet(imageClearDescriptorSet, imageClearDescriptorWrites);

			widthPrevious  = swapchainWidth;
			heightPrevious = swapchainHeight;

			clearMeanImage = true;
		}

		cameraManager.update(dt);

		const vkcv::CommandStreamHandle cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		const auto fullscreenDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(swapchainWidth, swapchainHeight),
				vkcv::DispatchSize(8, 8)
		);

		if (updateMaterials) {
			std::vector<Material> materials;
			for (const auto& settings : materialSettings) {
				materials.push_back(settings.second);
			}
			materialBuffer.fill(materials);
			updateMaterials = false;
			clearMeanImage  = true;
		}

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
				{ vkcv::useDescriptorSet(0, imageClearDescriptorSet) },
				vkcv::PushConstants(0));

			clearMeanImage = false;
		}

		// path tracing
		struct RaytracingPushConstantData {
			glm::mat4   viewToWorld;
			glm::vec3   skyColor;
			int32_t     sphereCount;
			int32_t     planeCount;
			int32_t     frameIndex;
		};

		RaytracingPushConstantData raytracingPushData;
		raytracingPushData.viewToWorld  = glm::inverse(cameraManager.getActiveCamera().getView());
		raytracingPushData.skyColor     = skyColor * skyColorMultiplier;
		raytracingPushData.sphereCount  = spheres.size();
		raytracingPushData.planeCount   = planes.size();
		raytracingPushData.frameIndex   = frameIndex;

		vkcv::PushConstants pushConstantsCompute = vkcv::pushConstants<RaytracingPushConstantData>();
		pushConstantsCompute.appendDrawcall(raytracingPushData);
		
		const auto traceDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(swapchainWidth, swapchainHeight),
				vkcv::DispatchSize(16, 16)
		);

		core.prepareImageForStorage(cmdStream, outputImage);

		core.recordComputeDispatchToCmdStream(cmdStream,
			tracePipeline,
			traceDispatchCount,
			{ vkcv::useDescriptorSet(0, traceDescriptorSet) },
			pushConstantsCompute);

		core.prepareImageForStorage(cmdStream, meanImage);
		core.recordImageMemoryBarrier(cmdStream, outputImage);

		// combine images
		core.recordComputeDispatchToCmdStream(cmdStream,
			imageCombinePipeline,
			fullscreenDispatchCount,
			{ vkcv::useDescriptorSet(0, imageCombineDescriptorSet) },
			vkcv::PushConstants(0));

		core.recordImageMemoryBarrier(cmdStream, meanImage);

		// present image
		const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

		vkcv::DescriptorWrites presentDescriptorWrites;
		presentDescriptorWrites.writeStorageImage(
				0, meanImage
		).writeStorageImage(
				1, swapchainInput
		);
		
		core.writeDescriptorSet(presentDescriptorSet, presentDescriptorWrites);

		core.prepareImageForStorage(cmdStream, swapchainInput);

		core.recordComputeDispatchToCmdStream(cmdStream,
			presentPipeline,
			fullscreenDispatchCount,
			{ vkcv::useDescriptorSet(0, presentDescriptorSet) },
			vkcv::PushConstants(0));

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);

		if (renderUI) {
			gui.beginGUI();

			ImGui::Begin("Settings");

			clearMeanImage |= ImGui::ColorEdit3("Sky color", &skyColor.x);
			clearMeanImage |= ImGui::InputFloat("Sky color multiplier", &skyColorMultiplier);

			if (ImGui::CollapsingHeader("Materials")) {

				for (auto& setting : materialSettings) {
					if (ImGui::CollapsingHeader(setting.first.c_str())) {

						const glm::vec3 emission            = setting.second.emission;
						float           emissionStrength    = glm::max(glm::max(glm::max(emission.x, emission.y), emission.z), 1.f);
						glm::vec3       emissionColor       = emission / emissionStrength;

						updateMaterials |= ImGui::ColorEdit3((std::string("Emission color ")    + setting.first).c_str(), &emissionColor.x);
						updateMaterials |= ImGui::InputFloat((std::string("Emission strength ") + setting.first).c_str(), &emissionStrength);

						setting.second.emission = emissionStrength * emissionColor;

						updateMaterials |= ImGui::ColorEdit3((std::string("Albedo color ")  + setting.first).c_str(), &setting.second.albedo.x);
						updateMaterials |= ImGui::ColorEdit3((std::string("F0 ")            + setting.first).c_str(), &setting.second.f0.x);
						updateMaterials |= ImGui::DragFloat(( std::string("ks ")            + setting.first).c_str(), &setting.second.ks, 0.01, 0, 1);
						updateMaterials |= ImGui::DragFloat(( std::string("roughness ")     + setting.first).c_str(), &setting.second.roughness, 0.01, 0, 1);

					}
				}
			}

			ImGui::End();

			gui.endGUI();
		}

		frameIndex++;
	});

	return 0;
}
