#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <chrono>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "RTX/RTX.hpp"
#include "RTX/RTXExtensions.hpp"
#include "teapot.hpp"

/**
 * Note: This project is based on the following tutorial https://github.com/Apress/Ray-Tracing-Gems-II/tree/main/Chapter_16.
 */

int main(int argc, const char** argv) {
	const std::string applicationName = "RTX Ambient Occlusion";

	// prepare raytracing extensions. IMPORTANT: configure compiler to build in 64 bit mode
	vkcv::rtx::RTXExtensions rtxExtensions;
	std::vector<const char*> raytracingInstanceExtensions = rtxExtensions.getInstanceExtensions();

	std::vector<const char*> instanceExtensions = {};   // add some more instance extensions, if needed
	instanceExtensions.insert(instanceExtensions.end(), raytracingInstanceExtensions.begin(), raytracingInstanceExtensions.end());  // merge together all instance extensions

	vkcv::Features features = rtxExtensions.getFeatures();  // all features required by the RTX device extensions
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
			features,
			instanceExtensions
	);

	vkcv::rtx::ASManager asManager(&core);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 800, 600, true);

	vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
	auto camHandle = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camHandle).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camHandle).setNearFar(0.1f, 30.0f);
	
    // get Teapot vertices and indices
    Teapot teapot;
    std::vector<float> vertices = teapot.getVertices();
    std::vector<uint32_t> indices = teapot.getIndices();

	vkcv::shader::GLSLCompiler compiler (vkcv::shader::GLSLCompileTarget::RAY_TRACING);

	vkcv::ShaderProgram rtxShaderProgram;
	compiler.compile(vkcv::ShaderStage::RAY_GEN, std::filesystem::path("resources/shaders/ambientOcclusion.rgen"),
		[&rtxShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
            rtxShaderProgram.addShader(shaderStage, path);
		});

	compiler.compile(vkcv::ShaderStage::RAY_CLOSEST_HIT, std::filesystem::path("resources/shaders/ambientOcclusion.rchit"),
		[&rtxShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
            rtxShaderProgram.addShader(shaderStage, path);
		});

	compiler.compile(vkcv::ShaderStage::RAY_MISS, std::filesystem::path("resources/shaders/ambientOcclusion.rmiss"),
		[&rtxShaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
            rtxShaderProgram.addShader(shaderStage, path);
		});

	std::vector<vkcv::DescriptorSetHandle> descriptorSetHandles;
	std::vector<vkcv::DescriptorSetLayoutHandle> descriptorSetLayoutHandles;

	vkcv::DescriptorSetLayoutHandle rtxShaderDescriptorSetLayout = core.createDescriptorSetLayout(rtxShaderProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle rtxShaderDescriptorSet = core.createDescriptorSet(rtxShaderDescriptorSetLayout);
	descriptorSetHandles.push_back(rtxShaderDescriptorSet);
	descriptorSetLayoutHandles.push_back(rtxShaderDescriptorSetLayout);

	// init RTXModule
	vkcv::rtx::RTXModule rtxModule(&core, &asManager, vertices, indices,descriptorSetHandles);

	struct RaytracingPushConstantData {
	    glm::vec4 camera_position;   // as origin for ray generation
	    glm::vec4 camera_right;      // for computing ray direction
	    glm::vec4 camera_up;         // for computing ray direction
	    glm::vec4 camera_forward;    // for computing ray direction
	};
	
	auto rtxPipeline = core.createRayTracingPipeline(vkcv::RayTracingPipelineConfig(
			rtxShaderProgram,
			descriptorSetLayoutHandles
	));

	vkcv::ImageHandle depthBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();

	vkcv::DescriptorWrites rtxWrites;
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		if ((!depthBuffer) ||
			(swapchainWidth != core.getImageWidth(depthBuffer)) ||
			((swapchainHeight != core.getImageHeight(depthBuffer)))) {
			depthBuffer = core.createImage(
					vk::Format::eD32Sfloat,
					vkcv::ImageConfig(
							swapchainWidth,
							swapchainHeight
					)
			);
		}
		
		cameraManager.update(dt);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		
		RaytracingPushConstantData raytracingPushData;
		raytracingPushData.camera_position = glm::vec4(cameraManager.getActiveCamera().getPosition(),0);
		raytracingPushData.camera_right = glm::vec4(glm::cross(cameraManager.getActiveCamera().getUp(), cameraManager.getActiveCamera().getFront()), 0);
		raytracingPushData.camera_up = glm::vec4(cameraManager.getActiveCamera().getUp(),0);
		raytracingPushData.camera_forward = glm::vec4(cameraManager.getActiveCamera().getFront(),0);

		vkcv::PushConstants pushConstantsRTX = vkcv::pushConstants<RaytracingPushConstantData>();
		pushConstantsRTX.appendDrawcall(raytracingPushData);

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		rtxWrites.writeStorageImage(0, swapchainInput);
		core.writeDescriptorSet(rtxShaderDescriptorSet, rtxWrites);

		core.prepareImageForStorage(cmdStream, swapchainInput);

		core.recordRayGenerationToCmdStream(
			cmdStream,
			rtxPipeline,
			vkcv::DispatchSize(swapchainWidth, swapchainHeight),
			{ vkcv::useDescriptorSet(0, rtxShaderDescriptorSet) },
			pushConstantsRTX,
			windowHandle
		);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
	});

	return 0;
}
