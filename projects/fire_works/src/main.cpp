
#include <array>

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
	float mass;
	glm::vec3 pad0;
	uint32_t eventId;
};

struct event_t {
	glm::vec3 direction;
	float startTime;
	glm::vec3 color;
	float velocity;
	
	uint32_t count;
	uint32_t index;
	uint32_t parent;
	uint32_t continuous;
	
	float lifetime;
	float mass;
	float size;
	uint32_t contCount;
};

struct smoke_t {
	glm::vec3 position;
	float size;
	glm::vec3 velocity;
	float scaling;
	glm::vec3 color;
	float eventID;
};

struct trail_t {
	uint32_t particleIndex;
	uint32_t startIndex;
	uint32_t endIndex;
	uint32_t useCount;
	glm::vec3 color;
	float lifetime;
};

struct point_t {
	glm::vec3 position;
	float size;
	glm::vec3 velocity;
	float scaling;
};

struct draw_particles_t {
	glm::mat4 mvp;
	uint32_t width;
	uint32_t height;
};

struct draw_smoke_t {
	glm::mat4 mvp;
	glm::vec3 camera;
};

#define PARTICLE_COUNT (1024)
#define SMOKE_COUNT (512)
#define TRAIL_COUNT (2048)
#define RANDOM_DATA_LENGTH (1024)
#define POINT_COUNT (2048 * 256)

void InitializeParticles(std::vector<particle_t> &particles) {
	for (size_t i = 0; i < particles.size(); i++) {
		particle_t particle;
		particle.position = glm::vec3(2.0f * (std::rand() % RAND_MAX) / RAND_MAX - 1.0f,
									  2.0f * (std::rand() % RAND_MAX) / RAND_MAX - 1.0f,
									  2.0f * (std::rand() % RAND_MAX) / RAND_MAX - 1.0f);

		particle.lifetime = 0.0f;
		particle.velocity = glm::vec3(0.0f);
		particle.size = 0.01f;
		particle.color = glm::vec3(1.0f, 0.0f, 0.0f);

		particles [i] = particle;
	}
}

void InitializeFireworkEvents(std::vector<event_t>& events) {
	events.emplace_back(glm::vec3(0, 1, 0), 0.5f, glm::vec3(0.0f, 1.0f, 0.0f), 12.5f,

						1, 0, UINT_MAX, 0,

						1.0f, 1.0f, 0.5f, 0);

	events.emplace_back(glm::vec3(0.0f), 1.5f, glm::vec3(0.0f, 1.0f, 1.0f), 10.0f,

						100, 0, events.size() - 1, 0,

						10.0f, 1.0f, 0.0f, 0);

	events.emplace_back(glm::vec3(0.5, 1, 0), 0.25f, glm::vec3(0.0f, 1.5f, 0.0f), 15.0f,

						1, 0, UINT_MAX, 0,

						0.5f, 1.0f, 0.5f, 0);

	events.emplace_back(glm::vec3(0.0f), 0.75f, glm::vec3(0.0f, 1.5f, 1.0f), 8.0f,

						150, 0, events.size() - 1, 0,

						10.0f, 1.0f, 0.0f, 0);

	events.emplace_back(glm::vec3(-2.5, 3, 0.5), 1.0f, glm::vec3(246.0f, 189.0f, 255.0f), 12.5f,

						1, 0, UINT_MAX, 0,

						1.0f, 1.0f, 0.5f, 0);

	events.emplace_back(glm::vec3(0.0f), 2.0f, glm::vec3(235.0f, 137.0f, 250.0f), 8.0f,

						75, 0, events.size() - 1, 0,

						10.0f, 1.0f, 0.0f, 0);
}

void InitializeSparklerEvents(std::vector<event_t> &events) {
	events.emplace_back(glm::vec3(0, 1, 0), 0.0f, glm::vec3(251.0f, 255.0f, 145.0f), 1.0f,

						1, 0, UINT_MAX, 0,

						8.0f, 0.0f, 0.5f, 0);

	events.emplace_back(glm::vec3(0.0f), 0.0f, glm::vec3(251.0f, 255.0f, 145.0f), 10.0f,

						1000, 1, events.size() - 1, 10,

						0.5f, -1.0f, 0.0f, 100);
}

void InitializeNestedFireworkEvents(std::vector<event_t>& events) {
	events.emplace_back(glm::vec3(0, 2, 0), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f), 12.5f,

						1, 0, UINT_MAX, 0,

						1.0f, 1.0f, 0.5f, 0);

	events.emplace_back(glm::vec3(0.0f), 0.9f, glm::vec3(0.0f, 1.0f, 1.0f), 7.0f,

						100, 0, events.size() - 1, 0,

						10.1f, 1.0f, 0.0f, 0);

	events.emplace_back(glm::vec3(0.0f), 2.0f, glm::vec3(0.0f, 0.0f, 0.0f), 10.0f,

						100, 0, events.size() - 1, 0,

						10.0f, 1.0f, 0.0f, 0);

	events.emplace_back(glm::vec3(0.0f), 1.0f, glm::vec3(42.0f,0.0f, 1.0f), 12.5f,

						100, 0, events.size() - 2, 0,

						1.0f, 1.0f, 0.5f, 0);

	events.emplace_back(glm::vec3(0.0f), 1.5f, glm::vec3(42.0f, 0.0f, 1.0f), 10.0f,

						100, 0, events.size() - 1, 0,

						10.0f, 1.0f, 0.0f, 0);

	events.emplace_back(glm::vec3(0.0f), 2.0f, glm::vec3(42.0f, 0.0f, 1.0f), 10.0f,

						100, 0, events.size() - 1, 0,

						10.0f, 1.0f, 0.0f, 0);
}

void ChangeColor(std::vector<event_t>& events, glm::vec3 color) {
	for (int i = 0; i < events.size(); i++) {
		events [i].color = color;
	}
}

int main(int argc, const char **argv) {
	vkcv::Features features;
	
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
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
	uint32_t pilotIdx = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
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
	vkcv::DescriptorBinding binding1 { 
		1,     
		vkcv::DescriptorType::STORAGE_BUFFER,
									   
		1,     
		vkcv::ShaderStage::COMPUTE,
		false, 
		false 
	};
	
	descriptorBindings0.insert(std::make_pair(0, binding0));
	descriptorBindings0.insert(std::make_pair(1, binding1));
	
	vkcv::DescriptorSetLayoutHandle descriptorSetLayout = core.createDescriptorSetLayout(descriptorBindings0);
	vkcv::DescriptorSetHandle descriptorSet = core.createDescriptorSet(descriptorSetLayout);
	
	vkcv::ShaderProgram generationShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/generation.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		generationShader.addShader(shaderStage, path);
	});
	
	auto generationBindings = generationShader.getReflectedDescriptors().at(1);
	generationBindings[0].shaderStages |= vkcv::ShaderStage::FRAGMENT;
	
	vkcv::DescriptorSetLayoutHandle generationDescriptorLayout = core.createDescriptorSetLayout(
		generationBindings
	);
	
	vkcv::DescriptorSetHandle generationDescriptorSet = core.createDescriptorSet(generationDescriptorLayout);
	
	vkcv::DescriptorBindings descriptorBindings1;
	
	descriptorBindings1.insert(std::make_pair(0, binding0));
	descriptorBindings1.insert(std::make_pair(1, binding1));
	
	vkcv::DescriptorSetLayoutHandle smokeDescriptorLayout = core.createDescriptorSetLayout(descriptorBindings1);
	vkcv::DescriptorSetHandle smokeDescriptorSet = core.createDescriptorSet(smokeDescriptorLayout);
	
	vkcv::DescriptorBindings descriptorBindings2;
	vkcv::DescriptorBinding binding2 {
		1,
		vkcv::DescriptorType::STORAGE_BUFFER,
		1,
		vkcv::ShaderStage::GEOMETRY | vkcv::ShaderStage::COMPUTE,
		false,
		false
	};
	
	descriptorBindings2.insert(std::make_pair(0, binding0));
	descriptorBindings2.insert(std::make_pair(1, binding2));
	
	vkcv::DescriptorSetLayoutHandle trailDescriptorLayout = core.createDescriptorSetLayout(
		descriptorBindings2
	);
	
	vkcv::DescriptorSetHandle trailDescriptorSet = core.createDescriptorSet(trailDescriptorLayout);
	
	vkcv::ComputePipelineHandle generationPipeline = core.createComputePipeline({
		generationShader,
		{
          descriptorSetLayout,
          generationDescriptorLayout,
          smokeDescriptorLayout,
          trailDescriptorLayout
		}
	});
	
	vkcv::ShaderProgram trailShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/trail.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		trailShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle trailComputePipeline = core.createComputePipeline({
		trailShader,
		{ descriptorSetLayout, trailDescriptorLayout }
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
	
	const vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
	
	std::array<vkcv::ImageHandle, 4> colorBuffers;
	for (size_t i = 0; i < colorBuffers.size(); i++) {
		colorBuffers[i] = core.createImage(
				colorFormat,
				swapchainExtent.width,
				swapchainExtent.height,
				1, false, true, true
		).getHandle();
	}
	
	vkcv::ShaderProgram particleShaderProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/particle.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		particleShaderProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "shaders/particle.frag", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		particleShaderProgram.addShader(shaderStage, path);
	});
	
	vkcv::ShaderProgram trailShaderProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/trail.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		trailShaderProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::GEOMETRY, "shaders/trail.geom", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		trailShaderProgram.addShader(shaderStage, path);
	});
	
	vkcv::ShaderProgram smokeShaderProgram;
	compiler.compile(vkcv::ShaderStage::VERTEX, "shaders/smoke.vert", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		smokeShaderProgram.addShader(shaderStage, path);
	});
	compiler.compile(vkcv::ShaderStage::FRAGMENT, "shaders/smoke.frag", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		smokeShaderProgram.addShader(shaderStage, path);
		trailShaderProgram.addShader(shaderStage, path);
	});
	
	std::vector<particle_t> particles;
	particles.resize(PARTICLE_COUNT);
	InitializeParticles(particles);
	
	vkcv::Buffer<particle_t> particleBuffer = core.createBuffer<particle_t>(
		vkcv::BufferType::STORAGE,
		particles.size(),
		vkcv::BufferMemoryType::DEVICE_LOCAL,
		false,
		true
	);
	
	particleBuffer.fill(particles);
	
	vkcv::Buffer<particle_t> particleBufferCopy =
		core.createBuffer<particle_t>(vkcv::BufferType::STORAGE, particles.size());

	particleBufferCopy.fill(particles);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, particleBuffer.getHandle());
		writes.writeStorageBuffer(1, particleBufferCopy.getHandle());
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
	InitializeFireworkEvents(events);
	
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

	vkcv::Buffer<uint32_t> startIndexBuffer =
		core.createBuffer<uint32_t>(vkcv::BufferType::STORAGE, eventBuffer.getCount());

	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(2, startIndexBuffer.getHandle());
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
	
	vkcv::Buffer<uint32_t> smokeIndexBuffer = core.createBuffer<uint32_t>(
		vkcv::BufferType::STORAGE, 3, vkcv::BufferMemoryType::HOST_VISIBLE
	);
	
	uint32_t* smokeIndices = smokeIndexBuffer.map();
	memset(smokeIndices, 0, smokeIndexBuffer.getSize());
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, smokeBuffer.getHandle());
		writes.writeStorageBuffer(1, smokeIndexBuffer.getHandle());
		core.writeDescriptorSet(smokeDescriptorSet, writes);
	}
	
	std::vector<trail_t> trails;
	trails.reserve(TRAIL_COUNT);
	
	for (size_t i = 0; i < TRAIL_COUNT; i++) {
		trail_t trail;
		
		trail.particleIndex = 0;
		trail.startIndex = 0;
		trail.endIndex = 0;
		trail.useCount = 0;
		trail.color = glm::vec3(0.0f);
		trail.lifetime = 0.0f;
		
		trails.push_back(trail);
	}
	
	vkcv::Buffer<trail_t> trailBuffer = core.createBuffer<trail_t>(
		vkcv::BufferType::STORAGE,
		trails.size()
	);
	
	trailBuffer.fill(trails);
	
	std::vector<point_t> points;
	points.reserve(POINT_COUNT);
	
	for (size_t i = 0; i < POINT_COUNT; i++) {
		point_t point;
		
		point.position = glm::vec3(0.0f);
		point.size = 0.0f;
		point.velocity = glm::vec3(0.0f);
		point.scaling = 0.0f;
		
		points.push_back(point);
	}
	
	vkcv::Buffer<point_t> pointBuffer = core.createBuffer<point_t>(
		vkcv::BufferType::STORAGE,
		points.size()
	);
	
	pointBuffer.fill(points);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageBuffer(0, trailBuffer.getHandle());
		writes.writeStorageBuffer(1, pointBuffer.getHandle());
		core.writeDescriptorSet(trailDescriptorSet, writes);
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
	
	vkcv::PassHandle renderPass = core.createPass(vkcv::PassConfig(
		{
			vkcv::AttachmentDescription(
				vkcv::AttachmentOperation::STORE,
				vkcv::AttachmentOperation::CLEAR,
				colorFormat
				)
		},
		vkcv::Multisampling::None
	));
	
	vkcv::GraphicsPipelineConfig smokePipelineDefinition{
		smokeShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		renderPass,
		{smokeLayout},
		{smokeDescriptorLayout, generationDescriptorLayout},
		true
	};
	
	smokePipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;
	
	vkcv::GraphicsPipelineHandle smokePipeline = core.createGraphicsPipeline(smokePipelineDefinition);
	
	const std::vector<vkcv::VertexAttachment> vaTrail = trailShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbTrail;
	for (size_t i = 0; i < vaTrail.size(); i++) {
		vbTrail.push_back(vkcv::createVertexBinding(i, { vaTrail[i] }));
	}
	
	const vkcv::VertexLayout trailLayout { vbTrail };
	
	vkcv::GraphicsPipelineConfig trailPipelineDefinition{
		trailShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		renderPass,
		{trailLayout},
		{trailDescriptorLayout, generationDescriptorLayout, descriptorSetLayout},
		true
	};
	
	trailPipelineDefinition.m_PrimitiveTopology = vkcv::PrimitiveTopology::PointList;
	trailPipelineDefinition.m_blendMode = vkcv::BlendMode::Additive;
	
	vkcv::GraphicsPipelineHandle trailPipeline = core.createGraphicsPipeline(trailPipelineDefinition);
	
	std::vector<vkcv::DrawcallInfo> drawcallsSmokes;
	
	drawcallsSmokes.push_back(vkcv::DrawcallInfo(
		cubeMesh,
		{
			vkcv::DescriptorSetUsage(0, smokeDescriptorSet),
			vkcv::DescriptorSetUsage(1, generationDescriptorSet),
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
	
	vkcv::Mesh trailMesh (
		{},
		triangleIndices.getVulkanHandle(),
		1
	);
	
	std::vector<vkcv::DrawcallInfo> drawcallsTrails;
	
	drawcallsTrails.push_back(vkcv::DrawcallInfo(
		trailMesh,
		{
			vkcv::DescriptorSetUsage(0, trailDescriptorSet),
			vkcv::DescriptorSetUsage(1, generationDescriptorSet),
			vkcv::DescriptorSetUsage(2, descriptorSet)
		},
		trailBuffer.getCount()
	));
	
	const std::vector<vkcv::VertexAttachment> vaParticles = particleShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbParticles;
	for (size_t i = 0; i < vaParticles.size(); i++) {
		vbParticles.push_back(vkcv::createVertexBinding(i, { vaParticles[i] }));
	}

	const vkcv::VertexLayout particleLayout { vbParticles };

	vkcv::GraphicsPipelineConfig particlePipelineDefinition{
		particleShaderProgram,
		UINT32_MAX,
		UINT32_MAX,
		renderPass,
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
	
	vkcv::ShaderProgram addShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/add.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		addShader.addShader(shaderStage, path);
	});
	
	vkcv::DescriptorSetLayoutHandle addDescriptorLayout = core.createDescriptorSetLayout(addShader.getReflectedDescriptors().at(0));
	vkcv::DescriptorSetHandle addDescriptor = core.createDescriptorSet(addDescriptorLayout);
	vkcv::ComputePipelineHandle addPipe = core.createComputePipeline({
		addShader,
		{ addDescriptorLayout }
	});
	
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
	
		for (size_t i = 0; i < colorBuffers.size(); i++) {
			if ((core.getImageWidth(colorBuffers[i]) != swapchainWidth) ||
				(core.getImageHeight(colorBuffers[i]) != swapchainHeight)) {
				colorBuffers[i] = core.createImage(
										   colorFormat,
										   swapchainWidth,
										   swapchainHeight,
										   1, false, true, true
										   ).getHandle();
			}
		}
		
		auto next = std::chrono::system_clock::now();
		
		auto time = std::chrono::duration_cast<std::chrono::microseconds>(next - start);
		auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(next - current);
		
		current = next;
		
		float time_values [2];
		time_values[0] = 0.000001f * static_cast<float>(time.count());
		time_values[1] = 0.000001f * static_cast<float>(deltatime.count());
		
		std::cout << time_values[0] << " " << time_values[1] << std::endl;
		
		auto cmdStream = core.createCommandStream(vkcv::QueueType::Graphics);
		
		core.recordBufferMemoryBarrier(cmdStream, eventBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, smokeBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, smokeIndexBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, trailBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, pointBuffer.getHandle());
		
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
				vkcv::DescriptorSetUsage(2, smokeDescriptorSet),
				vkcv::DescriptorSetUsage(3, trailDescriptorSet)
			},
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, smokeBuffer.getHandle());
		
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
		
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		
		core.recordBeginDebugLabel(cmdStream, "Particle motion", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			motionPipeline,
			particleDispatchCount,
			{ vkcv::DescriptorSetUsage(0, descriptorSet) },
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, trailBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, pointBuffer.getHandle());
		
		uint32_t trailDispatchCount[3];
		trailDispatchCount[0] = std::ceil(trailBuffer.getCount() / 256.f);
		trailDispatchCount[1] = 1;
		trailDispatchCount[2] = 1;
		
		core.recordBeginDebugLabel(cmdStream, "Trail update", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			trailComputePipeline,
			trailDispatchCount,
			{
				vkcv::DescriptorSetUsage(0, descriptorSet),
				vkcv::DescriptorSetUsage(1, trailDescriptorSet)
			},
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		cameraManager.update(time_values[1]);
		
		const auto& camera = cameraManager.getActiveCamera();
		
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		
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
			renderPass,
			particlePipeline,
			pushConstantsDraw0,
			{ drawcallsParticles },
			{ colorBuffers[0] },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, smokeBuffer.getHandle());
		
		draw_smoke_t draw_smoke {
			camera.getMVP(),
			camera.getPosition()
		};
		
		core.recordBeginDebugLabel(cmdStream, "Draw smoke", { 1.0f, 0.5f, 1.0f, 1.0f });
		vkcv::PushConstants pushConstantsDraw1 (sizeof(draw_smoke_t));
		pushConstantsDraw1.appendDrawcall(draw_smoke);
		
		core.recordDrawcallsToCmdStream(
			cmdStream,
			renderPass,
			smokePipeline,
			pushConstantsDraw1,
			{ drawcallsSmokes },
			{ colorBuffers[1] },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, trailBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, pointBuffer.getHandle());
		
		core.recordBeginDebugLabel(cmdStream, "Draw trails", { 0.75f, 0.5f, 1.0f, 1.0f });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			renderPass,
			trailPipeline,
			pushConstantsDraw1,
			{ drawcallsTrails },
			{ colorBuffers[2] },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Add rendered images", { 0.5f, 0.5f, 1.0f, 1.0f });
		
		vkcv::DescriptorWrites addDescriptorWrites;
		for (size_t i = 0; i < colorBuffers.size(); i++) {
			addDescriptorWrites.writeStorageImage(i, colorBuffers[i]);
			core.prepareImageForStorage(cmdStream, colorBuffers[i]);
		}
		
		core.writeDescriptorSet(addDescriptor, addDescriptorWrites);
		
		uint32_t colorDispatchCount[3];
		colorDispatchCount[0] = std::ceil(swapchainWidth / 8.f);
		colorDispatchCount[1] = std::ceil(swapchainHeight / 8.f);
		colorDispatchCount[2] = 1;
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			addPipe,
			colorDispatchCount,
			{vkcv::DescriptorSetUsage(0, addDescriptor) },
			vkcv::PushConstants(0)
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		bloomAndFlares.recordEffect(cmdStream, colorBuffers.back(), colorBuffers.back());
		
		core.recordBeginDebugLabel(cmdStream, "Tonemapping", { 0.0f, 1.0f, 0.0f, 1.0f });
		core.prepareImageForStorage(cmdStream, colorBuffers.back());
		core.prepareImageForStorage(cmdStream, swapchainImage);
		
		vkcv::DescriptorWrites tonemappingDescriptorWrites;
		tonemappingDescriptorWrites.writeStorageImage(
			0, colorBuffers.back()
		).writeStorageImage(
			1, swapchainImage
		);
		
		core.writeDescriptorSet(tonemappingDescriptor, tonemappingDescriptorWrites);
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			tonemappingPipe,
			colorDispatchCount,
			{vkcv::DescriptorSetUsage(0, tonemappingDescriptor) },
			vkcv::PushConstants(0)
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		ImGui::Begin("Settings");
		
		bool listbox = ImGui::BeginListBox(" ");
		bool firework = ImGui::Selectable("Firework");
		bool sparkler = ImGui::Selectable("Sparkler");
		bool nested = ImGui::Selectable("Nested Firework");
		ImGui::EndListBox();
		bool resetTime = ImGui::Button("Reset");
		auto color = glm::vec3(0.0f);
		
		if (!events.empty()) {
			color = events[0].color;
		}
		
		bool colorChanged = ImGui::ColorPicker3("Color", (float*) & color);
		
		ImGui::End();
		gui.endGUI();
		
		core.endFrame(windowHandle);

		particleBuffer.read(particles);
		sort(particles.begin(), particles.end(),
			 [](const particle_t p1, const particle_t p2) {
				 return p1.eventId < p2.eventId;
			 });

		std::vector<uint32_t> startingIndex;
		startingIndex.resize(events.size());
		uint32_t eventIdCheck = std::numeric_limits<uint32_t>::max();
		
		for (size_t i = 0; i < particles.size(); i++) {
			if (particles[i].eventId != eventIdCheck) {
				eventIdCheck = particles [i].eventId;
				if (eventIdCheck < startingIndex.size()) {
					startingIndex [eventIdCheck] = i;
				}
			}
		}

		startIndexBuffer.fill(startingIndex);

		if (firework) {
			events.clear();
			InitializeFireworkEvents(events);
			resetTime = true;
		} else if (sparkler) {
			events.clear();
			InitializeSparklerEvents(events);
			resetTime = true;
		} else if (nested) {
			events.clear();
			InitializeNestedFireworkEvents(events);
			resetTime = true;
		}

		if (colorChanged) {
			ChangeColor(events, color);
			resetTime = true;
		}
		
		if (resetTime) {
			start = std::chrono::system_clock::now();	
			InitializeParticles(particles);
			particleBuffer.fill(particles);
			eventBuffer.fill(events);
			smokeBuffer.fill(smokes);
			trailBuffer.fill(trails);
			pointBuffer.fill(points);
			
			memset(smokeIndices, 0, smokeIndexBuffer.getSize());
		}

		particleBufferCopy.fill(particles);
	}
	
	smokeIndexBuffer.unmap();
	return 0;
}
