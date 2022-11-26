#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/geometry/Teapot.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include "RTX/RTX.hpp"

/**
 * Note: This project is based on the following tutorial https://github.com/Apress/Ray-Tracing-Gems-II/tree/main/Chapter_16.
 */

int main(int argc, const char** argv) {
	const std::string applicationName = "RTX Ambient Occlusion";
	
	vkcv::Features features;
	features.requireExtension(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
	features.requireExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	features.requireExtension(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
	features.requireExtension(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
	features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			[](vk::PhysicalDeviceDescriptorIndexingFeatures& features) {}
	);
	
	features.requireExtensionFeature<vk::PhysicalDeviceBufferDeviceAddressFeatures>(
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			[](vk::PhysicalDeviceBufferDeviceAddressFeatures& features) {
				features.setBufferDeviceAddress(true);
			}
	);
	
	features.requireExtensionFeature<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			[](vk::PhysicalDeviceAccelerationStructureFeaturesKHR& features) {
				features.setAccelerationStructure(true);
			}
	);
	
	features.requireExtensionFeature<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>(
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			[](vk::PhysicalDeviceRayTracingPipelineFeaturesKHR& features) {
				features.setRayTracingPipeline(true);
			}
	);
	
	features.requireFeature<vk::PhysicalDevice16BitStorageFeatures>(
			[](vk::PhysicalDevice16BitStorageFeatures &features) {
				features.setStorageBuffer16BitAccess(true);
			}
	);
	
	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
			features,
			{ VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME }
	);

	vkcv::rtx::ASManager asManager(&core);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 800, 600, true);
	
	vkcv::geometry::Teapot teapot (glm::vec3(0.0f), 1.0f);
	vkcv::VertexData vertexData = teapot.generateVertexData(core);

	vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
	auto camHandle = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	
	cameraManager.getCamera(camHandle).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camHandle).setNearFar(0.1f, 30.0f);

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
	vkcv::rtx::RTXModule rtxModule (
			&core,
			&asManager,
			vertexData,
			descriptorSetHandles
	);

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
