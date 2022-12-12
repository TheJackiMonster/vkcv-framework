#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/denoising/ShadowDenoiser.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>
#include <vkcv/upscaling/FSRUpscaling.hpp>

/**
 * Note: This project is based on the following tutorial https://github.com/Apress/Ray-Tracing-Gems-II/tree/main/Chapter_16.
 */

struct ObjDesc {
	uint64_t vertexAddress;
	uint64_t indexAddress;
	uint32_t vertexStride;
	uint32_t pad0;
	uint32_t pad1;
	uint32_t pad2;
	glm::mat4 transform;
};

int main(int argc, const char** argv) {
	const std::string applicationName = "Ray Tracing: Ambient Occlusion";
	
	vkcv::Features features;
	features.requireExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
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
	
	features.requireFeature(
			[](vk::PhysicalDeviceFeatures &features) {
				features.setShaderInt64(true);
			}
	);
	
	vkcv::Core core = vkcv::Core::create(
			applicationName,
			VK_MAKE_VERSION(0, 0, 1),
			{ vk::QueueFlagBits::eGraphics ,vk::QueueFlagBits::eCompute , vk::QueueFlagBits::eTransfer },
			features
	);

	vkcv::WindowHandle windowHandle = core.createWindow(applicationName, 800, 600, true);
	
	vkcv::scene::Scene scene = vkcv::scene::Scene::load(
			core,
			"resources/Sponza/Sponza.gltf",
			{
					vkcv::asset::PrimitiveType::POSITION
			}
	);

	vkcv::camera::CameraManager cameraManager(core.getWindow(windowHandle));
	auto camHandle = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	auto camHandle2 = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(camHandle).setPosition(glm::vec3(0, 0, -3));
	cameraManager.getCamera(camHandle).setNearFar(0.1f, 30.0f);

	vkcv::shader::GLSLCompiler compiler (vkcv::shader::GLSLCompileTarget::RAY_TRACING);

	vkcv::ShaderProgram shaderProgram;
	compiler.compile(vkcv::ShaderStage::RAY_GEN, std::filesystem::path("resources/shaders/ambientOcclusion.rgen"),
		[&shaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
            shaderProgram.addShader(shaderStage, path);
		});

	compiler.compile(vkcv::ShaderStage::RAY_CLOSEST_HIT, std::filesystem::path("resources/shaders/ambientOcclusion.rchit"),
		[&shaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
            shaderProgram.addShader(shaderStage, path);
		});

	compiler.compile(vkcv::ShaderStage::RAY_MISS, std::filesystem::path("resources/shaders/ambientOcclusion.rmiss"),
		[&shaderProgram](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
            shaderProgram.addShader(shaderStage, path);
		});

	std::vector<vkcv::DescriptorSetHandle> descriptorSetHandles;
	std::vector<vkcv::DescriptorSetLayoutHandle> descriptorSetLayoutHandles;

	vkcv::DescriptorSetLayoutHandle shaderDescriptorSetLayout = core.createDescriptorSetLayout(shaderProgram.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle shaderDescriptorSet = core.createDescriptorSet(shaderDescriptorSetLayout);
	descriptorSetHandles.push_back(shaderDescriptorSet);
	descriptorSetLayoutHandles.push_back(shaderDescriptorSetLayout);
	
	const uint32_t instanceCount = scene.getMeshCount();
	const uint32_t geometryCount = scene.getMeshPartCount();
	
	std::vector<ObjDesc> objDescList;
	objDescList.resize(instanceCount * static_cast<size_t>(
			std::ceil(static_cast<float>(geometryCount) / instanceCount)
	));
	
	vkcv::AccelerationStructureHandle scene_tlas = scene.createAccelerationStructure(
			[&core, &objDescList, instanceCount](
					size_t instanceIndex,
					size_t geometryIndex,
					const vkcv::GeometryData &geometry,
					const glm::mat4 &transform
			) {
				ObjDesc obj {};
				
				obj.vertexAddress = core.getBufferDeviceAddress(
						geometry.getVertexBufferBinding().m_buffer
				);
				
				obj.indexAddress = core.getBufferDeviceAddress(
						geometry.getIndexBuffer()
				);
				
				obj.vertexStride = geometry.getVertexStride() / sizeof(float);
				obj.transform = transform;
				
				objDescList[geometryIndex * instanceCount + instanceIndex] = obj;
			}
	);
	
	auto objDescBuffer = vkcv::buffer<ObjDesc>(
			core,
			vkcv::BufferType::STORAGE,
			objDescList.size()
	);
	
	objDescBuffer.fill(objDescList);
	
	auto contextBuffer = vkcv::buffer<uint32_t>(
			core, vkcv::BufferType::STORAGE, 2
	);
	
	uint32_t* context = contextBuffer.map();
	
	context[0] = instanceCount;
	context[1] = 16;
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeAcceleration(1, { scene_tlas });
		writes.writeStorageBuffer(2, objDescBuffer.getHandle());
		writes.writeStorageBuffer(3, contextBuffer.getHandle());
		core.writeDescriptorSet(descriptorSetHandles[0], writes);
	}
	
	vkcv::denoising::ShadowDenoiser denoiser (core);
	vkcv::upscaling::FSRUpscaling upscaling (core);
	
	uint32_t fsrWidth = core.getWindow(windowHandle).getWidth();
	uint32_t fsrHeight = core.getWindow(windowHandle).getHeight();
	
	vkcv::upscaling::FSRQualityMode fsrMode = vkcv::upscaling::FSRQualityMode::NONE;
	int fsrModeIndex = static_cast<int>(fsrMode);
	
	const std::vector<const char*> fsrModeNames = {
			"None",
			"Ultra Quality",
			"Quality",
			"Balanced",
			"Performance"
	};

	struct RaytracingPushConstantData {
	    glm::vec4 camera_position;   // as origin for ray generation
	    glm::vec4 camera_right;      // for computing ray direction
	    glm::vec4 camera_up;         // for computing ray direction
	    glm::vec4 camera_forward;    // for computing ray direction
	};
	
	auto pipeline = core.createRayTracingPipeline(vkcv::RayTracingPipelineConfig(
			shaderProgram,
			descriptorSetLayoutHandles
	));
	
	const vk::Format depthBufferFormat = vk::Format::eD32Sfloat;
	const vk::Format colorBufferFormat = vk::Format::eR16G16B16A16Sfloat;
	
	vkcv::ImageConfig depthBufferConfig (
			fsrWidth,
			fsrHeight
	);
	
	vkcv::ImageHandle depthBuffer = core.createImage(
			depthBufferFormat,
			depthBufferConfig
	);
	
	vkcv::ImageConfig colorBufferConfig (
			fsrWidth,
			fsrHeight
	);
	
	colorBufferConfig.setSupportingStorage(true);
	colorBufferConfig.setSupportingColorAttachment(true);
	
	vkcv::ImageHandle colorBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
	vkcv::gui::GUI gui (core, windowHandle);
	
	core.run([&](const vkcv::WindowHandle &windowHandle, double t, double dt,
				 uint32_t swapchainWidth, uint32_t swapchainHeight) {
		uint32_t width, height;
		vkcv::upscaling::getFSRResolution(
				fsrMode,
				swapchainWidth, swapchainHeight,
				width, height
		);
		
		if ((!colorBuffer) || (!depthBuffer) ||
			(width != fsrWidth) || (height != fsrHeight)) {
			fsrWidth = width;
			fsrHeight = height;
			
			depthBufferConfig.setWidth(fsrWidth);
			depthBufferConfig.setHeight(fsrHeight);
			
			depthBuffer = core.createImage(
					depthBufferFormat,
					depthBufferConfig
			);
			
			colorBufferConfig.setWidth(fsrWidth);
			colorBufferConfig.setHeight(fsrHeight);
			
			colorBuffer = core.createImage(
					colorBufferFormat,
					colorBufferConfig
			);
		}
		
		if ((!colorBuffer) || (!depthBuffer)) {
			return;
		}
		
		cameraManager.update(dt);

		const std::vector<vkcv::ImageHandle> renderTargets = { swapchainInput, depthBuffer };
		
		const glm::mat4 view = glm::transpose(cameraManager.getActiveCamera().getView());
		
		RaytracingPushConstantData raytracingPushData;
		raytracingPushData.camera_position = glm::vec4(cameraManager.getActiveCamera().getPosition(), 0);
		raytracingPushData.camera_right = glm::vec4(glm::vec3(view[0]), 0);
		raytracingPushData.camera_up = glm::vec4(glm::vec3(view[1]), 0);
		raytracingPushData.camera_forward = glm::vec4(glm::vec3(view[2]), 0);

		vkcv::PushConstants pushConstants = vkcv::pushConstants<RaytracingPushConstantData>();
		pushConstants.appendDrawcall(raytracingPushData);
		
		{
			vkcv::DescriptorWrites writes;
			writes.writeStorageImage(0, colorBuffer);
			core.writeDescriptorSet(shaderDescriptorSet, writes);
		}

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.prepareImageForStorage(cmdStream, colorBuffer);

		core.recordRayGenerationToCmdStream(
			cmdStream,
			pipeline,
			vkcv::DispatchSize(width, height),
			{ vkcv::useDescriptorSet(0, shaderDescriptorSet) },
			pushConstants,
			windowHandle
		);
		
		core.prepareImageForStorage(cmdStream, colorBuffer);
		
		denoiser.recordDenoising(cmdStream, colorBuffer, colorBuffer);
		
		core.prepareImageForSampling(cmdStream, colorBuffer);
		core.prepareImageForStorage(cmdStream, swapchainInput);
		
		upscaling.recordUpscaling(cmdStream, colorBuffer, swapchainInput);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		
		ImGui::Begin("Settings");
		
		int sampleCount = static_cast<int>(context[1]);
		
		ImGui::SliderInt("Samples", &sampleCount, 1, 128);
		
		if (static_cast<uint32_t>(sampleCount) != context[1]) {
			context[1] = static_cast<uint32_t>(sampleCount);
		}
		
		float sharpness = upscaling.getSharpness();
		
		ImGui::Combo("FSR Quality Mode", &fsrModeIndex, fsrModeNames.data(), fsrModeNames.size());
		ImGui::DragFloat("FSR Sharpness", &sharpness, 0.001, 0.0f, 1.0f);
		
		if ((fsrModeIndex >= 0) && (fsrModeIndex <= 4)) {
			fsrMode = static_cast<vkcv::upscaling::FSRQualityMode>(fsrModeIndex);
		}
		
		upscaling.setSharpness(sharpness);
		
		ImGui::End();
		
		gui.endGUI();
	});
	
	contextBuffer.unmap();
	return 0;
}
