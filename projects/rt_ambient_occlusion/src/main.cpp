#include <vkcv/Core.hpp>
#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/scene/Scene.hpp>

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
	
	auto instanceCountBuffer = vkcv::buffer<uint32_t>(core, vkcv::BufferType::STORAGE, 1);
	instanceCountBuffer.fill(&instanceCount);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeAcceleration(1, { scene_tlas });
		writes.writeStorageBuffer(2, objDescBuffer.getHandle());
		writes.writeStorageBuffer(3, instanceCountBuffer.getHandle());
		core.writeDescriptorSet(descriptorSetHandles[0], writes);
	}

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

	vkcv::ImageHandle depthBuffer;

	const vkcv::ImageHandle swapchainInput = vkcv::ImageHandle::createSwapchainImageHandle();
	
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
			writes.writeStorageImage(0, swapchainInput);
			core.writeDescriptorSet(shaderDescriptorSet, writes);
		}

		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);

		core.prepareImageForStorage(cmdStream, swapchainInput);

		core.recordRayGenerationToCmdStream(
			cmdStream,
			pipeline,
			vkcv::DispatchSize(swapchainWidth, swapchainHeight),
			{ vkcv::useDescriptorSet(0, shaderDescriptorSet) },
			pushConstants,
			windowHandle
		);

		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
	});

	return 0;
}
