
#include <vkcv/Core.hpp>
#include <vkcv/DrawcallRecording.hpp>

#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/effects/BloomAndFlaresEffect.hpp>

struct particle_t {
	glm::vec3 position;
	float lifetime;
	glm::vec3 velocity;
	float size;
	glm::vec3 color;
	float pad2;
};

struct event_t {
	glm::vec3 direction;
	float startTime;
	glm::vec3 color;
	float velocity;
	
	uint count;
	uint index;
	uint parent;
	uint pad0;
	
	float lifetime;
	float mass;
	float size;
	float pad1;
};

struct draw_particles_t {
	glm::mat4 mvp;
	uint width;
	uint height;
};

int main(int argc, const char **argv) {
	vkcv::Core core = vkcv::Core::create(
		"Firework",
		VK_MAKE_VERSION(0, 0, 1),
		{vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
		{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
	);
	
	vkcv::WindowHandle windowHandle = core.createWindow("Firework", 800, 600, true);
	vkcv::Window& window = core.getWindow (windowHandle);
	vkcv::camera::CameraManager cameraManager (window);
	
	uint32_t trackballIdx = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	cameraManager.getCamera(trackballIdx).setCenter(glm::vec3(0.0f, 0.0f, 0.0f));   // set camera to look at the center of the particle volume
	uint pilotIdx = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(trackballIdx).setNearFar(0.1f, 50.0f);
	cameraManager.getCamera(trackballIdx).setPosition(glm::vec3(0, 0, -25));
	
	cameraManager.getCamera(pilotIdx).setNearFar(0.1f, 50.0f);
	cameraManager.getCamera(pilotIdx).setPosition(glm::vec3(0, 0, 25));
	
	cameraManager.setActiveCamera(pilotIdx);
	
	vkcv::gui::GUI gui (core, windowHandle);
	vkcv::shader::GLSLCompiler compiler;
	
	vkcv::DescriptorBindings descriptorBindings;
	vkcv::DescriptorBinding binding {
		0,
		vkcv::DescriptorType::STORAGE_BUFFER,
		1,
		vkcv::ShaderStage::VERTEX | vkcv::ShaderStage::COMPUTE,
		false,
		false
	};
	
	descriptorBindings.insert(std::make_pair(0, binding));
	
	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);
	
	vkcv::ShaderProgram generationShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/generation.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		generationShader.addShader(shaderStage, path);
	});
	
	vkcv::DescriptorSetLayoutHandle generationDescriptorLayout = core.createDescriptorSetLayout(generationShader.getReflectedDescriptors().at(1));
	vkcv::DescriptorSetHandle generationDescriptorSet = core.createDescriptorSet(generationDescriptorLayout);
	vkcv::ComputePipelineHandle generationPipeline = core.createComputePipeline({
		generationShader,
		{ descriptorSetLayout, generationDescriptorLayout }
	});
	
	auto swapchainExtent = core.getSwapchain(windowHandle).getExtent();
	
	const vk::Format depthFormat = vk::Format::eD32Sfloat;
	
	vkcv::ImageHandle depthBuffer = core.createImage(
		depthFormat,
		swapchainExtent.width,
		swapchainExtent.height
	).getHandle();
	
	const vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
	
	vkcv::ImageHandle colorBuffer = core.createImage(
		colorFormat,
		swapchainExtent.width,
		swapchainExtent.height,
		1, false, true, true
	).getHandle();
	
	vkcv::ShaderProgram particleShaderProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/particle.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		particleShaderProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "shaders/particle.frag", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		particleShaderProgram.addShader(shaderStage, path);
	});
	
	std::vector<particle_t> particles;
	
	for (size_t i = 0; i < 1024; i++) {
		particle_t particle;
		particle.position = glm::vec3(
			2.0f * (std::rand() % RAND_MAX) / RAND_MAX - 1.0f,
			2.0f * (std::rand() % RAND_MAX) / RAND_MAX - 1.0f,
			2.0f * (std::rand() % RAND_MAX) / RAND_MAX - 1.0f
		);
		
		particle.lifetime = 0.0f;
		particle.velocity = glm::vec3(0.0f);
		particle.size = 0.01f;
		particle.color = glm::vec3(1.0f, 0.0f, 0.0f);
		
		particles.push_back(particle);
	}
	
	vkcv::Buffer<particle_t> particleBuffer = core.createBuffer<particle_t>(
		vkcv::BufferType::STORAGE,
		particles.size()
	);
	
	particleBuffer.fill(particles);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, particleBuffer.getHandle());
		core.writeDescriptorSet(descriptorSet, writes);
	}
	
	std::vector<float> randomData;
	for (size_t i = 0; i < 1024; i++) {
		randomData.push_back(
			2.0f * static_cast<float>(std::rand() % RAND_MAX) / static_cast<float>(RAND_MAX) - 1.0f
		);
	}
	
	vkcv::Buffer<float> randomBuffer = core.createBuffer<float>(
		vkcv::BufferType::STORAGE,
		randomData.size()
	);
	
	randomBuffer.fill(randomData);
	
	std::vector<event_t> events;
	
	events.emplace_back(
		glm::vec3(0, 1, 0),
		0.0f,
		glm::vec3(0.0f, 1.0f, 0.0f),
		12.5f,
		
		1,
		0,
		UINT_MAX,
		0,
		
		1.0f,
		1.0f,
		0.5f,
		0.0f
	);
	
	events.emplace_back(
		glm::vec3(0.0f),
		1.0f,
		glm::vec3(0.0f, 1.0f, 1.0f),
		10.0f,
		
		100,
		0,
		events.size() - 1,
		0,
		
		10.0f,
		1.0f,
		0.0f,
		0.0f
	);
	
	vkcv::Buffer<event_t> eventBuffer = core.createBuffer<event_t>(
		vkcv::BufferType::STORAGE,
		events.size()
	);
	
	eventBuffer.fill(events);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, randomBuffer.getHandle());
		writes.writeStorageBuffer(1, eventBuffer.getHandle());
		core.writeDescriptorSet(generationDescriptorSet, writes);
	}
	
	vkcv::Buffer<glm::vec2> trianglePositions = core.createBuffer<glm::vec2>(vkcv::BufferType::VERTEX, 3);
	trianglePositions.fill({
		glm::vec2(-1.0f, -1.0f),
		glm::vec2(+0.0f, +1.5f),
		glm::vec2(+1.0f, -1.0f)
	});
	
	vkcv::Buffer<uint16_t> triangleIndices = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 3);
	triangleIndices.fill({
		0, 1, 2
	});
	
	vkcv::Mesh triangleMesh (
		{ vkcv::VertexBufferBinding(0, trianglePositions.getVulkanHandle()) },
		triangleIndices.getVulkanHandle(),
		triangleIndices.getCount()
	);
	
	const std::vector<vkcv::VertexAttachment> vertexAttachments = particleShaderProgram.getVertexAttachments();
	
	const std::vector<vkcv::VertexBufferBinding> vertexBufferBindings = {
		vkcv::VertexBufferBinding(0, trianglePositions.getVulkanHandle())
	};
	
	std::vector<vkcv::VertexBinding> bindings;
	for (size_t i = 0; i < vertexAttachments.size(); i++) {
		bindings.push_back(vkcv::createVertexBinding(i, {vertexAttachments[i]}));
	}

	const vkcv::VertexLayout particleLayout { bindings };
	
	const vkcv::AttachmentDescription present_color_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		colorFormat
	);
	
	const vkcv::AttachmentDescription depth_attachment(
		vkcv::AttachmentOperation::STORE,
		vkcv::AttachmentOperation::CLEAR,
		depthFormat
	);
	
	vkcv::PassConfig particlePassDefinition({present_color_attachment, depth_attachment}, vkcv::Multisampling::None);
	vkcv::PassHandle particlePass = core.createPass(particlePassDefinition);

	vkcv::GraphicsPipelineConfig particlePipelineDefinition{
		particleShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		particlePass,
		{particleLayout},
		{descriptorSetLayout},
		true
	};
	
	// particlePipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;
	
	vkcv::GraphicsPipelineHandle particlePipeline = core.createGraphicsPipeline(particlePipelineDefinition);
	
	std::vector<vkcv::DrawcallInfo> drawcallsParticles;
	
	drawcallsParticles.push_back(vkcv::DrawcallInfo(
		triangleMesh,
		{ vkcv::DescriptorSetUsage(0, descriptorSet) },
		particleBuffer.getCount()
	));
	
	vkcv::ShaderProgram motionShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/motion.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		motionShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle motionPipeline = core.createComputePipeline({
		motionShader,
		{ descriptorSetLayout }
	});
	
	vkcv::effects::BloomAndFlaresEffect bloomAndFlares (core);
	bloomAndFlares.setUpsamplingLimit(3);
	
	vkcv::ShaderProgram tonemappingShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/tonemapping.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		tonemappingShader.addShader(shaderStage, path);
	});
	
	vkcv::DescriptorSetLayoutHandle tonemappingDescriptorLayout = core.createDescriptorSetLayout(tonemappingShader.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle tonemappingDescriptor = core.createDescriptorSet(tonemappingDescriptorLayout);
	vkcv::ComputePipelineHandle tonemappingPipe = core.createComputePipeline({
		tonemappingShader,
		{ tonemappingDescriptorLayout }
	});
	
	vkcv::ImageHandle swapchainImage = vkcv::ImageHandle::createSwapchainImageHandle();
	
	auto start = std::chrono::system_clock::now();
	auto current = start;
	
	while (vkcv::Window::hasOpenWindow()) {
		vkcv::Window::pollEvents();
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
			continue;
		}
	
		if ((core.getImageWidth(colorBuffer) != swapchainWidth) ||
			(core.getImageHeight(colorBuffer) != swapchainHeight)) {
			colorBuffer = core.createImage(
				  colorFormat,
				  swapchainWidth,
				  swapchainHeight,
				  1, false, true, true
		  	).getHandle();
		}
		
		if ((core.getImageWidth(depthBuffer) != swapchainWidth) ||
			(core.getImageHeight(depthBuffer) != swapchainHeight)) {
			depthBuffer = core.createImage(
				  depthFormat,
				  swapchainWidth,
				  swapchainHeight
			).getHandle();
		}
		
		auto next = std::chrono::system_clock::now();
		
		auto time = std::chrono::duration_cast<std::chrono::microseconds>(next - start);
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(next - current);
		
		current = next;
		
		float time_values [2];
		time_values[0] = static_cast<float>(0.000001 * static_cast<double>(time.count()));
		time_values[1] = static_cast<float>(0.000001 * static_cast<double>(deltatime.count()));
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		uint32_t particleDispatchCount[3];
		particleDispatchCount[0] = std::ceil(particleBuffer.getCount() / 256.f);
		particleDispatchCount[1] = 1;
		particleDispatchCount[2] = 1;
		
		vkcv::PushConstants pushConstantsTime (2 * sizeof(float));
		pushConstantsTime.appendDrawcall(time_values);
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			generationPipeline,
			particleDispatchCount,
			{
				vkcv::DescriptorSetUsage(0, descriptorSet),
				vkcv::DescriptorSetUsage(1, generationDescriptorSet)
			},
			pushConstantsTime
		);
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			motionPipeline,
			particleDispatchCount,
			{ vkcv::DescriptorSetUsage(0, descriptorSet) },
			pushConstantsTime
		);
		
		cameraManager.update(time_values[1]);
		
		const auto& camera = cameraManager.getActiveCamera();
		
		draw_particles_t draw_particles {
			camera.getMVP(),
			swapchainWidth,
			swapchainHeight
		};
		
		vkcv::PushConstants pushConstantsDraw (sizeof(draw_particles_t));
		pushConstantsDraw.appendDrawcall(draw_particles);
		
		core.recordDrawcallsToCmdStream(
			cmdStream,
			particlePass,
			particlePipeline,
			pushConstantsDraw,
			{ drawcallsParticles },
			{ colorBuffer, depthBuffer },
			windowHandle
		);
		
		bloomAndFlares.recordEffect(cmdStream, colorBuffer, colorBuffer);
		
		core.prepareImageForStorage(cmdStream, colorBuffer);
		core.prepareImageForStorage(cmdStream, swapchainImage);
		
		vkcv::DescriptorWrites tonemappingDescriptorWrites;
		tonemappingDescriptorWrites.writeStorageImage(
			0, colorBuffer
		).writeStorageImage(
			1, swapchainImage
		);
		
		core.writeDescriptorSet(tonemappingDescriptor, tonemappingDescriptorWrites);

		uint32_t tonemappingDispatchCount[3];
		tonemappingDispatchCount[0] = std::ceil(swapchainWidth / 8.f);
		tonemappingDispatchCount[1] = std::ceil(swapchainHeight / 8.f);
		tonemappingDispatchCount[2] = 1;

		core.recordComputeDispatchToCmdStream(
			cmdStream,
			tonemappingPipe,
			tonemappingDispatchCount,
			{vkcv::DescriptorSetUsage(0, tonemappingDescriptor) },
			vkcv::PushConstants(0)
		);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		ImGui::Begin("Settings");
		
		if (ImGui::Button("Reset")) {
			start = std::chrono::system_clock::now();
			
			particleBuffer.fill(particles);
			eventBuffer.fill(events);
		}
		
		ImGui::End();
		gui.endGUI();
		
		core.endFrame(windowHandle);
	}
	
	return 0;
}
