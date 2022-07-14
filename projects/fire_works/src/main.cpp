
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

struct smoke_t {
	glm::vec3 position;
	float size;
	glm::vec3 velocity;
	float scaling;
	glm::vec3 color;
	float pad;
};

struct draw_particles_t {
	glm::mat4 mvp;
	uint width;
	uint height;
};

#define PARTICLE_COUNT 1024
#define SMOKE_COUNT 256
#define SMOKE_RESOLUTION 16
#define RANDOM_DATA_LENGTH 1024

int main(int argc, const char **argv) {
	vkcv::Features features;
	
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, [](vk::PhysicalDeviceDescriptorIndexingFeatures &features) {
			features.setShaderInputAttachmentArrayDynamicIndexing(true);
			features.setShaderUniformTexelBufferArrayDynamicIndexing(true);
			features.setShaderStorageTexelBufferArrayDynamicIndexing(true);
			features.setShaderUniformBufferArrayNonUniformIndexing(true);
			features.setShaderSampledImageArrayNonUniformIndexing(true);
			features.setShaderStorageBufferArrayNonUniformIndexing(true);
			features.setShaderStorageImageArrayNonUniformIndexing(true);
			features.setShaderInputAttachmentArrayNonUniformIndexing(true);
			features.setShaderUniformTexelBufferArrayNonUniformIndexing(true);
			features.setShaderStorageTexelBufferArrayNonUniformIndexing(true);
			
			features.setDescriptorBindingUniformBufferUpdateAfterBind(true);
			features.setDescriptorBindingSampledImageUpdateAfterBind(true);
			features.setDescriptorBindingStorageImageUpdateAfterBind(true);
			features.setDescriptorBindingStorageBufferUpdateAfterBind(true);
			features.setDescriptorBindingUniformTexelBufferUpdateAfterBind(true);
			features.setDescriptorBindingStorageTexelBufferUpdateAfterBind(true);

			features.setDescriptorBindingUpdateUnusedWhilePending(true);
			features.setDescriptorBindingPartiallyBound(true);
			features.setDescriptorBindingVariableDescriptorCount(true);
			features.setRuntimeDescriptorArray(true);
		}
	);
	
	vkcv::Core core = vkcv::Core::create(
		"Firework",
		VK_MAKE_VERSION(0, 0, 1),
		{vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics, vk::QueueFlagBits::eCompute},
		features
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
	
	vkcv::DescriptorBindings descriptorBindings0;
	vkcv::DescriptorBinding binding0 {
		0,
		vkcv::DescriptorType::STORAGE_BUFFER,
		1,
		vkcv::ShaderStage::VERTEX | vkcv::ShaderStage::COMPUTE,
		false,
		false
	};
	
	descriptorBindings0.insert(std::make_pair(0, binding0));
	
	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings0);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);
	
	vkcv::ShaderProgram generationShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/generation.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		generationShader.addShader(shaderStage, path);
	});
	
	vkcv::DescriptorSetLayoutHandle generationDescriptorLayout = core.createDescriptorSetLayout(
		generationShader.getReflectedDescriptors().at(1)
	);
	
	vkcv::DescriptorSetHandle generationDescriptorSet = core.createDescriptorSet(generationDescriptorLayout);
	
	vkcv::DescriptorBindings descriptorBindings1;
	vkcv::DescriptorBinding binding1 {
		1,
		vkcv::DescriptorType::STORAGE_BUFFER,
		1,
		vkcv::ShaderStage::COMPUTE,
		false,
		false
	};
	
	descriptorBindings1.insert(std::make_pair(0, binding0));
	descriptorBindings1.insert(std::make_pair(1, binding1));
	
	vkcv::DescriptorSetLayoutHandle smokeDescriptorLayout = core.createDescriptorSetLayout(descriptorBindings1);
	vkcv::DescriptorSetHandle smokeDescriptorSet = core.createDescriptorSet(smokeDescriptorLayout);
	
	vkcv::ComputePipelineHandle generationPipeline = core.createComputePipeline({
		generationShader,
		{ descriptorSetLayout, generationDescriptorLayout, smokeDescriptorLayout }
	});
	
	vkcv::ShaderProgram scaleShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/scale.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		scaleShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle scalePipeline = core.createComputePipeline({
		scaleShader,
		{ smokeDescriptorLayout }
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
	
	vkcv::ShaderProgram smokeShaderProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/smoke.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		smokeShaderProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "shaders/smoke.frag", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		smokeShaderProgram.addShader(shaderStage, path);
	});
	
	auto smokeImagesBindings = smokeShaderProgram.getReflectedDescriptors().at(1);
	smokeImagesBindings[1].descriptorCount = SMOKE_COUNT;
	
	smokeImagesBindings[0].shaderStages |= vkcv::ShaderStage::COMPUTE;
	smokeImagesBindings[1].shaderStages |= vkcv::ShaderStage::COMPUTE;
	
	vkcv::DescriptorSetLayoutHandle smokeImagesDescriptorLayout = core.createDescriptorSetLayout(smokeImagesBindings);
	vkcv::DescriptorSetHandle smokeImagesDescriptorSet = core.createDescriptorSet(smokeImagesDescriptorLayout);
	
	vkcv::ShaderProgram fluidShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/fluid.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		fluidShader.addShader(shaderStage, path);
	});
	
	vkcv::DescriptorSetLayoutHandle fluidImageDescriptorSetLayout = core.createDescriptorSetLayout(
		fluidShader.getReflectedDescriptors().at(1)
	);
	
	vkcv::ComputePipelineHandle fluidPipeline = core.createComputePipeline({
		fluidShader,
		{
			smokeImagesDescriptorLayout,
			fluidImageDescriptorSetLayout
		}
	});
	
	std::vector<particle_t> particles;
	particles.reserve(PARTICLE_COUNT);
	
	for (size_t i = 0; i < PARTICLE_COUNT; i++) {
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
	randomData.reserve(RANDOM_DATA_LENGTH);
	
	for (size_t i = 0; i < RANDOM_DATA_LENGTH; i++) {
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
	
	std::vector<smoke_t> smokes;
	smokes.reserve(SMOKE_COUNT);
	
	for (size_t i = 0; i < SMOKE_COUNT; i++) {
		smoke_t smoke;
		smoke.position = glm::vec3(0.0f);
		smoke.size = 0.0f;
		
		smoke.velocity = glm::vec3(0.0f);
		smoke.scaling = 0.0f;
		
		smoke.color = glm::vec3(0.0f);
		
		smokes.push_back(smoke);
	}
	
	vkcv::Buffer<smoke_t> smokeBuffer = core.createBuffer<smoke_t>(
		vkcv::BufferType::STORAGE,
		smokes.size()
	);
	
	smokeBuffer.fill(smokes);
	
	vkcv::Buffer<uint> smokeIndexBuffer = core.createBuffer<uint>(
		vkcv::BufferType::STORAGE, 1, vkcv::BufferMemoryType::HOST_VISIBLE
	);
	
	uint* smokeIndex = smokeIndexBuffer.map();
	*smokeIndex = 0;
	smokeIndexBuffer.unmap();
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, smokeBuffer.getHandle());
		writes.writeStorageBuffer(1, smokeIndexBuffer.getHandle());
		core.writeDescriptorSet(smokeDescriptorSet, writes);
	}
	
	std::vector<vkcv::ImageHandle> smokeImages;
	smokeImages.reserve(smokes.size());
	
	std::vector<uint64_t> imageData;
	imageData.resize(SMOKE_RESOLUTION * SMOKE_RESOLUTION * SMOKE_RESOLUTION, 0);
	
	for (size_t i = 0; i < smokes.size(); i++) {
		vkcv::Image image = core.createImage(
			vk::Format::eR16G16B16A16Sfloat,
			SMOKE_RESOLUTION,
			SMOKE_RESOLUTION,
			SMOKE_RESOLUTION,
			false,
			true
		);
		
		image.fill(imageData.data());
		
		smokeImages.push_back(image.getHandle());
	}
	
	std::vector<vkcv::ImageHandle> fluidImages;
	fluidImages.reserve(smokeImages.size());
	
	for (size_t i = 0; i < smokeImages.size(); i++) {
		vkcv::Image image = core.createImage(
			vk::Format::eR16G16B16A16Sfloat,
			SMOKE_RESOLUTION,
			SMOKE_RESOLUTION,
			SMOKE_RESOLUTION,
			false,
			true
		);
		
		fluidImages.push_back(image.getHandle());
	}
	
	vkcv::SamplerHandle smokeSampler = core.createSampler(
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerFilterType::LINEAR,
		vkcv::SamplerMipmapMode::LINEAR,
		vkcv::SamplerAddressMode::CLAMP_TO_EDGE
	);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeSampler(0, smokeSampler);
		core.writeDescriptorSet(smokeImagesDescriptorSet, writes);
	}
	
	std::vector<vkcv::DescriptorSetHandle> fluidImagesDescriptorSets;
	fluidImagesDescriptorSets.reserve(smokeImages.size());
	
	for (size_t i = 0; i < smokeImages.size(); i++) {
		fluidImagesDescriptorSets.push_back(core.createDescriptorSet(
			fluidImageDescriptorSetLayout
		));
	}
	
	vkcv::Buffer<glm::vec3> cubePositions = core.createBuffer<glm::vec3>(vkcv::BufferType::VERTEX, 8);
	cubePositions.fill({
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(+1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, +1.0f, -1.0f),
		glm::vec3(+1.0f, +1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, +1.0f),
		glm::vec3(+1.0f, -1.0f, +1.0f),
		glm::vec3(-1.0f, +1.0f, +1.0f),
		glm::vec3(+1.0f, +1.0f, +1.0f)
	});
	
	vkcv::Buffer<uint16_t> cubeIndices = core.createBuffer<uint16_t>(vkcv::BufferType::INDEX, 36);
	cubeIndices.fill({
		0, 2, 3,
		0, 3, 1,
		1, 3, 7,
		1, 7, 5,
		
		5, 7, 6,
		5, 6, 4,
		4, 6, 2,
		4, 2, 0,
		
		2, 6, 7,
		2, 7, 3,
		1, 5, 4,
		1, 4, 0
	});
	
	vkcv::Mesh cubeMesh (
		{ vkcv::VertexBufferBinding(0, cubePositions.getVulkanHandle()) },
		cubeIndices.getVulkanHandle(),
		cubeIndices.getCount()
	);
	
	const std::vector<vkcv::VertexAttachment> vaSmoke = smokeShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbSmoke;
	for (size_t i = 0; i < vaSmoke.size(); i++) {
		vbSmoke.push_back(vkcv::createVertexBinding(i, { vaSmoke[i] }));
	}
	
	const vkcv::VertexLayout smokeLayout { vbSmoke };
	
	vkcv::PassHandle smokePass = core.createPass(vkcv::PassConfig(
		{
			vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::LOAD,
				colorFormat
			),
			vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::DONT_CARE,
				vkcv::AttachmentOperation::LOAD,
				depthFormat
			)
		},
		vkcv::Multisampling::None
	));
	
	vkcv::GraphicsPipelineConfig smokePipelineDefinition{
		smokeShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		smokePass,
		{smokeLayout},
		{smokeDescriptorLayout, smokeImagesDescriptorLayout},
		true
	};
	
	smokePipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;
	
	vkcv::GraphicsPipelineHandle smokePipeline = core.createGraphicsPipeline(smokePipelineDefinition);
	
	std::vector<vkcv::DrawcallInfo> drawcallsSmokes;
	
	drawcallsSmokes.push_back(vkcv::DrawcallInfo(
		cubeMesh,
		{
			vkcv::DescriptorSetUsage(0, smokeDescriptorSet),
			vkcv::DescriptorSetUsage(1, smokeImagesDescriptorSet)
		},
		smokeBuffer.getCount()
	));
	
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
	
	const std::vector<vkcv::VertexAttachment> vaParticles = particleShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbParticles;
	for (size_t i = 0; i < vaParticles.size(); i++) {
		vbParticles.push_back(vkcv::createVertexBinding(i, { vaParticles[i] }));
	}

	const vkcv::VertexLayout particleLayout { vbParticles };
	
	vkcv::PassHandle particlePass = core.createPass(vkcv::PassConfig(
		{
			vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::CLEAR,
				colorFormat
			),
			vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::CLEAR,
				depthFormat
			)
		},
		vkcv::Multisampling::None
	));

	vkcv::GraphicsPipelineConfig particlePipelineDefinition{
		particleShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		particlePass,
		{particleLayout},
		{descriptorSetLayout},
		true
	};
	
	particlePipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;
	
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
		
		{
			vkcv::DescriptorWrites writes;
		
			for (size_t i = 0; i < smokeImages.size(); i++) {
				writes.writeSampledImage(1, smokeImages[i], 0, false, i);
			}
		
			core.writeDescriptorSet(smokeImagesDescriptorSet, writes);
		}
		
		for (size_t i = 0; i < smokeImages.size(); i++) {
			vkcv::DescriptorWrites writes;
			writes.writeStorageImage(0, fluidImages[i]);
			core.writeDescriptorSet(fluidImagesDescriptorSets[i], writes);
		}
		
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
		
		core.recordBeginDebugLabel(cmdStream, "Generation", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			generationPipeline,
			particleDispatchCount,
			{
				vkcv::DescriptorSetUsage(0, descriptorSet),
				vkcv::DescriptorSetUsage(1, generationDescriptorSet),
				vkcv::DescriptorSetUsage(2, smokeDescriptorSet)
			},
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		uint32_t smokeDispatchCount[3];
		smokeDispatchCount[0] = std::ceil(smokeBuffer.getCount() / 256.f);
		smokeDispatchCount[1] = 1;
		smokeDispatchCount[2] = 1;
		
		core.recordBeginDebugLabel(cmdStream, "Smoke scaling", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			scalePipeline,
			smokeDispatchCount,
			{ vkcv::DescriptorSetUsage(0, smokeDescriptorSet) },
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Particle motion", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			motionPipeline,
			particleDispatchCount,
			{ vkcv::DescriptorSetUsage(0, descriptorSet) },
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		uint32_t fluidDispatchCount[3];
		fluidDispatchCount[0] = std::ceil(SMOKE_RESOLUTION / 4.f);
		fluidDispatchCount[1] = std::ceil(SMOKE_RESOLUTION / 4.f);
		fluidDispatchCount[2] = std::ceil(SMOKE_RESOLUTION / 4.f);
		
		core.recordBeginDebugLabel(cmdStream, "Flow simulation", { 0.1f, 0.5f, 1.0f, 1.0f });
		for (size_t i = 0; i < smokeImages.size(); i++) {
			float fluid_values [2];
			reinterpret_cast<uint*>(fluid_values)[0] = static_cast<uint>(i);
			fluid_values[1] = time_values[1];
			
			vkcv::PushConstants pushConstantsFluid (2 * sizeof(float));
			pushConstantsFluid.appendDrawcall(fluid_values);
			
			core.prepareImageForSampling(cmdStream, smokeImages[i]);
			core.prepareImageForStorage(cmdStream, fluidImages[i]);
			
			core.recordComputeDispatchToCmdStream(
				cmdStream,
				fluidPipeline,
				fluidDispatchCount,
				{
					vkcv::DescriptorSetUsage(0, smokeImagesDescriptorSet),
					vkcv::DescriptorSetUsage(1, fluidImagesDescriptorSets[i])
				},
				pushConstantsFluid
			);
		}
		core.recordEndDebugLabel(cmdStream);
		
		cameraManager.update(time_values[1]);
		
		const auto& camera = cameraManager.getActiveCamera();
		
		draw_particles_t draw_particles {
			camera.getMVP(),
			swapchainWidth,
			swapchainHeight
		};
		
		vkcv::PushConstants pushConstantsDraw0 (sizeof(draw_particles_t));
		pushConstantsDraw0.appendDrawcall(draw_particles);
		
		core.recordBeginDebugLabel(cmdStream, "Draw particles", { 1.0f, 0.0f, 1.0f, 1.0f });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			particlePass,
			particlePipeline,
			pushConstantsDraw0,
			{ drawcallsParticles },
			{ colorBuffer, depthBuffer },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Draw smoke", { 1.0f, 0.5f, 1.0f, 1.0f });
		vkcv::PushConstants pushConstantsDraw1 (sizeof(glm::mat4));
		pushConstantsDraw1.appendDrawcall(camera.getMVP());
		
		core.recordDrawcallsToCmdStream(
			cmdStream,
			smokePass,
			smokePipeline,
			pushConstantsDraw1,
			{ drawcallsSmokes },
			{ colorBuffer, depthBuffer },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		bloomAndFlares.recordEffect(cmdStream, colorBuffer, colorBuffer);
		
		core.recordBeginDebugLabel(cmdStream, "Tonemapping", { 0.0f, 1.0f, 0.0f, 1.0f });
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
		
		core.recordEndDebugLabel(cmdStream);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		ImGui::Begin("Settings");
		
		if (ImGui::Button("Reset")) {
			start = std::chrono::system_clock::now();
			
			particleBuffer.fill(particles);
			eventBuffer.fill(events);
			smokeBuffer.fill(smokes);
		}
		
		ImGui::End();
		gui.endGUI();
		
		core.endFrame(windowHandle);
		
		for (size_t i = 0; i < smokeImages.size(); i++) {
			std::swap(smokeImages[i], fluidImages[i]);
		}
	}
	
	return 0;
}
