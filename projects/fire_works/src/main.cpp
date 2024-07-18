
#include <array>

#include <vkcv/Buffer.hpp>
#include <vkcv/Core.hpp>
#include <vkcv/Image.hpp>
#include <vkcv/Pass.hpp>
#include <vkcv/Sampler.hpp>

#include <vkcv/camera/CameraManager.hpp>
#include <vkcv/effects/BloomAndFlaresEffect.hpp>
#include <vkcv/effects/GammaCorrectionEffect.hpp>
#include <vkcv/gui/GUI.hpp>
#include <vkcv/shader/GLSLCompiler.hpp>
#include <vkcv/tone/ReinhardToneMapping.hpp>

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
#define RANDOM_DATA_LENGTH (4096)
#define POINT_COUNT (2048 * 256)

void InitializeParticles(std::vector<particle_t> &particles) {
	const auto rand_max = static_cast<float>(RAND_MAX);
	for (auto& particle : particles) {
		particle.position = glm::vec3(2.0f * (std::rand() % RAND_MAX) / rand_max - 1.0f,
									  2.0f * (std::rand() % RAND_MAX) / rand_max - 1.0f,
									  2.0f * (std::rand() % RAND_MAX) / rand_max - 1.0f);

		particle.lifetime = 0.0f;
		particle.velocity = glm::vec3(0.0f);
		particle.size = 0.01f;
		particle.color = glm::vec3(1.0f, 0.0f, 0.0f);
	}
}

void InitializeFireworkEvents(std::vector<event_t>& events) {
	events.push_back({
		glm::vec3(0, 1, 0), 0.5f, glm::vec3(0.0f, 1.0f, 0.0f), 12.5f,
		1, 0, UINT_MAX, 0,
		1.0f, 1.0f, 0.5f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 1.5f, glm::vec3(0.0f, 1.0f, 1.0f), 10.0f,
		100, 0, 0, 0,
		10.0f, 1.0f, 0.0f, 0
	});

	events.push_back({
		glm::vec3(0.5, 1, 0), 0.25f, glm::vec3(0.0f, 1.5f, 0.0f), 15.0f,
		1, 0, UINT_MAX, 0,
		0.5f, 1.0f, 0.5f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 0.75f, glm::vec3(0.0f, 1.5f, 1.0f), 8.0f,
		150, 0, 2, 0,
		10.0f, 1.0f, 0.0f, 0
	});

	events.push_back({
		glm::vec3(-2.5, 3, 0.5), 1.0f, glm::vec3(246.0f, 189.0f, 255.0f), 12.5f,
		1, 0, UINT_MAX, 0,
		1.0f, 1.0f, 0.5f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 2.0f, glm::vec3(235.0f, 137.0f, 250.0f), 8.0f,
		75, 0, 4, 0,
		10.0f, 1.0f, 0.0f, 0
	});
}

void InitializeSparklerEvents(std::vector<event_t> &events) {
	events.push_back({
		glm::vec3(0, 1, 0), 0.0f, glm::vec3(251.0f, 255.0f, 145.0f), 1.0f,
		1, 0, UINT_MAX, 0,
		8.0f, 0.0f, 0.5f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 0.0f, glm::vec3(251.0f, 255.0f, 145.0f), 10.0f,
		1000, 1, 0, 10,
		0.5f, -1.0f, 0.0f, 100
	});
}

void InitializeNestedFireworkEvents(std::vector<event_t>& events) {
	events.push_back({
		glm::vec3(0, 2, 0), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f), 12.5f,
		1, 0, UINT_MAX, 0,
		1.0f, 1.0f, 0.5f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 0.9f, glm::vec3(0.0f, 1.0f, 1.0f), 7.0f,
		100, 0, 0, 0,
		10.1f, 1.0f, 0.0f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 2.0f, glm::vec3(0.0f, 0.0f, 0.0f), 10.0f,
		100, 0, 1, 0,
		10.0f, 1.0f, 0.0f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 1.0f, glm::vec3(42.0f,0.0f, 1.0f), 12.5f,
		100, 0, 1, 0,
		1.0f, 1.0f, 0.5f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 1.5f, glm::vec3(42.0f, 0.0f, 1.0f), 10.0f,
		100, 0, 3, 0,
		10.0f, 1.0f, 0.0f, 0
	});

	events.push_back({
		glm::vec3(0.0f), 2.0f, glm::vec3(42.0f, 0.0f, 1.0f), 10.0f,
		100, 0, 4, 0,
		10.0f, 1.0f, 0.0f, 0
	});
}

void ChangeColor(std::vector<event_t>& events, glm::vec3 color) {
	for (size_t i = 0; i < events.size(); i++) {
		events [i].color = color;
	}
}

int main(int argc, const char **argv) {
	vkcv::Features features;
	
	features.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	features.requireFeature([](vk::PhysicalDeviceFeatures& features) {
		features.setGeometryShader(true);
	});

	features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			[](vk::PhysicalDeviceDescriptorIndexingFeatures& features) {
				features.setDescriptorBindingPartiallyBound(true);
				features.setDescriptorBindingVariableDescriptorCount(true);
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
	
	auto trackballHandle = cameraManager.addCamera(vkcv::camera::ControllerType::TRACKBALL);
	auto pilotHandle = cameraManager.addCamera(vkcv::camera::ControllerType::PILOT);
	
	cameraManager.getCamera(trackballHandle).setCenter(glm::vec3(0.0f, 0.0f, 0.0f));   // set camera to look at the center of the particle volume
	cameraManager.getCamera(trackballHandle).setNearFar(0.1f, 50.0f);
	cameraManager.getCamera(trackballHandle).setPosition(glm::vec3(0, 0, 25));
	
	cameraManager.getCamera(pilotHandle).setNearFar(0.1f, 50.0f);
	cameraManager.getCamera(pilotHandle).setPosition(glm::vec3(0, 0, 25));
	
	cameraManager.setActiveCamera(pilotHandle);
	
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
	
	auto swapchainHandle = core.getWindow(windowHandle).getSwapchain();
	auto swapchainExtent = core.getSwapchainExtent(swapchainHandle);
	
	const vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
	
	std::array<vkcv::ImageHandle, 4> colorBuffers;
	for (size_t i = 0; i < colorBuffers.size(); i++) {
		vkcv::ImageConfig colorBufferConfig (
				swapchainExtent.width,
				swapchainExtent.height
		);
		
		colorBufferConfig.setSupportingStorage(true);
		colorBufferConfig.setSupportingColorAttachment(true);
		
		colorBuffers[i] = core.createImage(
				colorFormat,
				colorBufferConfig
		);
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
	
	auto particleBuffer = vkcv::buffer<particle_t>(
			core,
			vkcv::BufferType::STORAGE,
			particles.size(),
			vkcv::BufferMemoryType::DEVICE_LOCAL,
			true
	);
	
	particleBuffer.fill(particles);
	
	auto particleBufferCopy = vkcv::buffer<particle_t>(
			core,
			vkcv::BufferType::STORAGE,
			particles.size()
	);

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
	
	auto randomBuffer = vkcv::buffer<float>(
			core,
			vkcv::BufferType::STORAGE,
			randomData.size()
	);
	
	randomBuffer.fill(randomData);
	
	std::vector<event_t> events;
	InitializeFireworkEvents(events);
	
	auto eventBuffer = vkcv::buffer<event_t>(
			core,
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

	auto startIndexBuffer = vkcv::buffer<uint32_t>(
			core,
			vkcv::BufferType::STORAGE,
			eventBuffer.getCount()
	);

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
	
	auto smokeBuffer = vkcv::buffer<smoke_t>(
			core,
			vkcv::BufferType::STORAGE,
			smokes.size()
	);
	
	smokeBuffer.fill(smokes);
	
	auto smokeIndexBuffer = vkcv::buffer<uint32_t>(
			core,
			vkcv::BufferType::STORAGE,
			3,
			vkcv::BufferMemoryType::HOST_VISIBLE
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
	
	auto trailBuffer = vkcv::buffer<trail_t>(
			core,
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
	
	auto pointBuffer = vkcv::buffer<point_t>(
			core,
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
	
	auto cubePositions = vkcv::buffer<glm::vec3>(
			core,
			vkcv::BufferType::VERTEX,
			8
	);
	
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
	
	auto cubeIndices = vkcv::buffer<uint16_t>(
			core,
			vkcv::BufferType::INDEX,
			36
	);
	
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
	
	vkcv::VertexData cubeData ({ vkcv::vertexBufferBinding(cubePositions) });
	cubeData.setIndexBuffer(cubeIndices.getHandle());
	cubeData.setCount(cubeIndices.getCount());
	
	const std::vector<vkcv::VertexAttachment> vaSmoke = smokeShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbSmoke;
	for (size_t i = 0; i < vaSmoke.size(); i++) {
		vbSmoke.push_back(vkcv::createVertexBinding(i, { vaSmoke[i] }));
	}
	
	const vkcv::VertexLayout smokeLayout { vbSmoke };
	
	vkcv::PassHandle renderPass = vkcv::passFormat(core, colorFormat);
	
	vkcv::GraphicsPipelineConfig smokePipelineDefinition (
		smokeShaderProgram,
		renderPass,
		{smokeLayout},
		{smokeDescriptorLayout, generationDescriptorLayout}
	);
	
	smokePipelineDefinition.setBlendMode(vkcv::BlendMode::Additive);
	
	vkcv::GraphicsPipelineHandle smokePipeline = core.createGraphicsPipeline(smokePipelineDefinition);
	
	const std::vector<vkcv::VertexAttachment> vaTrail = trailShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbTrail;
	for (size_t i = 0; i < vaTrail.size(); i++) {
		vbTrail.push_back(vkcv::createVertexBinding(i, { vaTrail[i] }));
	}
	
	const vkcv::VertexLayout trailLayout { vbTrail };
	
	vkcv::GraphicsPipelineConfig trailPipelineDefinition (
		trailShaderProgram,
		renderPass,
		{trailLayout},
		{trailDescriptorLayout, generationDescriptorLayout, descriptorSetLayout}
	);
	
	trailPipelineDefinition.setPrimitiveTopology(vkcv::PrimitiveTopology::PointList);
	trailPipelineDefinition.setBlendMode(vkcv::BlendMode::Additive);
	
	vkcv::GraphicsPipelineHandle trailPipeline = core.createGraphicsPipeline(trailPipelineDefinition);
	
	vkcv::InstanceDrawcall drawcallSmoke (cubeData, smokeBuffer.getCount());
	drawcallSmoke.useDescriptorSet(0, smokeDescriptorSet);
	drawcallSmoke.useDescriptorSet(1, generationDescriptorSet);
	
	auto trianglePositions = vkcv::buffer<glm::vec2>(
			core,
			vkcv::BufferType::VERTEX,
			3
	);
	
	trianglePositions.fill({
		glm::vec2(-1.0f, -1.0f),
		glm::vec2(+0.0f, +1.5f),
		glm::vec2(+1.0f, -1.0f)
	});
	
	auto triangleIndices = vkcv::buffer<uint16_t>(
			core,
			vkcv::BufferType::INDEX,
			3
	);
	
	triangleIndices.fill({
		0, 1, 2
	});
	
	vkcv::VertexData triangleData ({ vkcv::vertexBufferBinding(trianglePositions) });
	triangleData.setIndexBuffer(triangleIndices.getHandle());
	triangleData.setCount(triangleIndices.getCount());
	
	vkcv::VertexData trailData;
	triangleData.setIndexBuffer(triangleIndices.getHandle());
	trailData.setCount(1);
	
	vkcv::InstanceDrawcall drawcallTrail (trailData, trailBuffer.getCount());
	drawcallTrail.useDescriptorSet(0, trailDescriptorSet);
	drawcallTrail.useDescriptorSet(1, generationDescriptorSet);
	drawcallTrail.useDescriptorSet(2, descriptorSet);
	
	const std::vector<vkcv::VertexAttachment> vaParticles = particleShaderProgram.getVertexAttachments();
	
	std::vector<vkcv::VertexBinding> vbParticles;
	for (size_t i = 0; i < vaParticles.size(); i++) {
		vbParticles.push_back(vkcv::createVertexBinding(i, { vaParticles[i] }));
	}

	const vkcv::VertexLayout particleLayout { vbParticles };

	vkcv::GraphicsPipelineConfig particlePipelineDefinition (
		particleShaderProgram,
		renderPass,
		{particleLayout},
		{descriptorSetLayout}
	);
	
	particlePipelineDefinition.setBlendMode(vkcv::BlendMode::Additive);
	
	vkcv::GraphicsPipelineHandle particlePipeline = core.createGraphicsPipeline(particlePipelineDefinition);
	
	vkcv::InstanceDrawcall drawcallParticle (triangleData, particleBuffer.getCount());
	drawcallParticle.useDescriptorSet(0, descriptorSet);
	
	vkcv::ShaderProgram motionShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/motion.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		motionShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle motionPipeline = core.createComputePipeline({
		motionShader,
		{ descriptorSetLayout }
	});
	
	const uint32_t voxelWidth = 160;
	const uint32_t voxelHeight = 90;
	const uint32_t voxelDepth = 64;
	
	std::vector<uint32_t> zeroVoxel;
	zeroVoxel.resize(voxelWidth * voxelHeight * voxelDepth, 0);
	
	vkcv::ImageConfig voxelImageConfig (
			voxelWidth,
			voxelHeight,
			voxelDepth
	);
	
	voxelImageConfig.setSupportingStorage(true);
	
	vkcv::Image voxelRed = vkcv::image(
			core,
			vk::Format::eR32Uint,
			voxelImageConfig
	);
	
	vkcv::Image voxelGreen = vkcv::image(
			core,
			vk::Format::eR32Uint,
			voxelImageConfig
	);
	
	vkcv::Image voxelBlue = vkcv::image(
			core,
			vk::Format::eR32Uint,
			voxelImageConfig
	);
	
	vkcv::Image voxelDensity = vkcv::image(
			core,
			vk::Format::eR32Uint,
			voxelImageConfig
	);
	
	std::array<vkcv::ImageHandle, 2> voxelData {
		core.createImage(
			vk::Format::eR16G16B16A16Sfloat,
			voxelImageConfig
		),
		core.createImage(
			vk::Format::eR16G16B16A16Sfloat,
			voxelImageConfig
		)
	};
	
	vkcv::ImageConfig voxelSamplesConfig (
			voxelWidth,
			voxelHeight
	);
	
	voxelSamplesConfig.setSupportingStorage(true);
	
	vkcv::Image voxelSamples = vkcv::image(
			core,
			colorFormat,
			voxelSamplesConfig
   	);
	
	vkcv::SamplerHandle voxelSampler = vkcv::samplerLinear(core, true);
	
	vkcv::ShaderProgram voxelClearShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/clear.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelClearShader.addShader(shaderStage, path);
	});
	
	const auto& voxelBindings = voxelClearShader.getReflectedDescriptors().at(0);
	auto voxelDescriptorSetLayout = core.createDescriptorSetLayout(voxelBindings);
	
	vkcv::ComputePipelineHandle voxelClearPipeline = core.createComputePipeline({
		voxelClearShader,
		{ voxelDescriptorSetLayout }
	});
	
	vkcv::ShaderProgram voxelParticleShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/voxel_particle.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelParticleShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle voxelParticlePipeline = core.createComputePipeline({
		voxelParticleShader,
		{ descriptorSetLayout, voxelDescriptorSetLayout }
	});
	
	vkcv::ShaderProgram voxelSmokeShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/voxel_smoke.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelSmokeShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle voxelSmokePipeline = core.createComputePipeline({
		voxelSmokeShader,
		{ smokeDescriptorLayout, voxelDescriptorSetLayout }
	});
	
	vkcv::ShaderProgram voxelTrailShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/voxel_trail.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelTrailShader.addShader(shaderStage, path);
	});
	
	vkcv::ComputePipelineHandle voxelTrailPipeline = core.createComputePipeline({
		voxelTrailShader,
		{ trailDescriptorLayout, voxelDescriptorSetLayout }
	});
	
	vkcv::ShaderProgram voxelShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/voxel.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelShader.addShader(shaderStage, path);
	});
	
	const auto& voxelOutBindings = voxelShader.getReflectedDescriptors().at(1);
	auto voxelOutDescriptorSetLayout = core.createDescriptorSetLayout(voxelOutBindings);
	
	vkcv::ComputePipelineHandle voxelPipeline = core.createComputePipeline({
		voxelShader,
		{ voxelDescriptorSetLayout, voxelOutDescriptorSetLayout }
	});
	
	vkcv::ShaderProgram fluidShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/fluid.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		fluidShader.addShader(shaderStage, path);
	});
	
	const auto& fluidBindings = fluidShader.getReflectedDescriptors().at(0);
	auto fluidDescriptorSetLayout = core.createDescriptorSetLayout(fluidBindings);
	
	vkcv::ComputePipelineHandle fluidPipeline = core.createComputePipeline({
		fluidShader,
		{ fluidDescriptorSetLayout }
	});
	
	vkcv::ShaderProgram voxelSampleShader;
	compiler.compile(vkcv::ShaderStage::COMPUTE, "shaders/sample.comp", [&](vkcv::ShaderStage shaderStage, const std::filesystem::path& path) {
		voxelSampleShader.addShader(shaderStage, path);
	});
	
	const auto& sampleBindings = voxelSampleShader.getReflectedDescriptors().at(1);
	auto samplesDescriptorSetLayout = core.createDescriptorSetLayout(sampleBindings);
	
	vkcv::ComputePipelineHandle voxelSamplePipeline = core.createComputePipeline({
		voxelSampleShader,
		{ voxelOutDescriptorSetLayout, samplesDescriptorSetLayout }
	});
	
	auto voxelDescriptorSet = core.createDescriptorSet(voxelDescriptorSetLayout);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageImage(0, voxelRed.getHandle());
		writes.writeStorageImage(1, voxelGreen.getHandle());
		writes.writeStorageImage(2, voxelBlue.getHandle());
		writes.writeStorageImage(3, voxelDensity.getHandle());
		core.writeDescriptorSet(voxelDescriptorSet, writes);
	}
	
	auto voxelOutDescriptorSet = core.createDescriptorSet(voxelOutDescriptorSetLayout);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageImage(0, voxelData[0]);
		core.writeDescriptorSet(voxelOutDescriptorSet, writes);
	}
	
	std::array<vkcv::DescriptorSetHandle, 2> fluidDescriptorSet {
		core.createDescriptorSet(fluidDescriptorSetLayout),
		core.createDescriptorSet(fluidDescriptorSetLayout)
	};
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeSampledImage(0, voxelData[0]);
		writes.writeSampler(1, voxelSampler);
		writes.writeStorageImage(2, voxelData[1]);
		core.writeDescriptorSet(fluidDescriptorSet[0], writes);
	}
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeSampledImage(0, voxelData[1]);
		writes.writeSampler(1, voxelSampler);
		writes.writeStorageImage(2, voxelData[0]);
		core.writeDescriptorSet(fluidDescriptorSet[1], writes);
	}
	
	auto samplesDescriptorSet = core.createDescriptorSet(samplesDescriptorSetLayout);
	
	{
		vkcv::DescriptorWrites writes;
		writes.writeStorageImage(0, voxelSamples.getHandle());
		core.writeDescriptorSet(samplesDescriptorSet, writes);
	}
	
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
		{ addDescriptorLayout, generationDescriptorLayout }
	});
	
	vkcv::tone::ReinhardToneMapping toneMapping (core);
	vkcv::effects::GammaCorrectionEffect gammaCorrection (core);
	
	vkcv::ImageHandle swapchainImage = vkcv::ImageHandle::createSwapchainImageHandle();
	
	auto start = std::chrono::system_clock::now();
	auto current = start;
	
	while (vkcv::Window::hasOpenWindow()) {
		vkcv::Window::pollEvents();
		
		uint32_t swapchainWidth, swapchainHeight;
		if (!core.beginFrame(swapchainWidth, swapchainHeight, windowHandle)) {
			continue;
		}
	
		for (auto& colorBuffer : colorBuffers) {
			if ((core.getImageWidth(colorBuffer) != swapchainWidth) ||
				(core.getImageHeight(colorBuffer) != swapchainHeight)) {
				vkcv::ImageConfig colorBufferConfig (
						swapchainWidth,
						swapchainHeight
				);
				
				colorBufferConfig.setSupportingStorage(true);
				colorBufferConfig.setSupportingColorAttachment(true);
				
				colorBuffer = core.createImage(
					colorFormat,
					colorBufferConfig
				);
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
		
		auto voxelDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(voxelWidth, voxelHeight, voxelDepth),
				vkcv::DispatchSize(4, 4, 4)
		);
		
		core.recordBeginDebugLabel(cmdStream, "Voxel clear", { 0.5f, 0.25f, 0.8f, 1.0f });
		core.prepareImageForStorage(cmdStream, voxelRed.getHandle());
		core.prepareImageForStorage(cmdStream, voxelGreen.getHandle());
		core.prepareImageForStorage(cmdStream, voxelBlue.getHandle());
		core.prepareImageForStorage(cmdStream, voxelDensity.getHandle());
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			voxelClearPipeline,
			voxelDispatchCount,
			{ vkcv::useDescriptorSet(0, voxelDescriptorSet) },
			vkcv::PushConstants(0)
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, eventBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, smokeBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, smokeIndexBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, trailBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, pointBuffer.getHandle());
		
		auto particleDispatchCount = vkcv::dispatchInvocations(particleBuffer.getCount(), 256);
		
		vkcv::PushConstants pushConstantsTime (2 * sizeof(float));
		pushConstantsTime.appendDrawcall(time_values);
		
		core.recordBeginDebugLabel(cmdStream, "Generation", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			generationPipeline,
			particleDispatchCount,
			{
				vkcv::useDescriptorSet(0, descriptorSet),
				vkcv::useDescriptorSet(1, generationDescriptorSet),
				vkcv::useDescriptorSet(2, smokeDescriptorSet),
				vkcv::useDescriptorSet(3, trailDescriptorSet)
			},
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, smokeBuffer.getHandle());
		
		auto smokeDispatchCount = vkcv::dispatchInvocations(smokeBuffer.getCount(), 256);
		
		core.recordBeginDebugLabel(cmdStream, "Smoke scaling", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			scalePipeline,
			smokeDispatchCount,
			{ vkcv::useDescriptorSet(0, smokeDescriptorSet) },
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		
		core.recordBeginDebugLabel(cmdStream, "Particle motion", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			motionPipeline,
			particleDispatchCount,
			{ vkcv::useDescriptorSet(0, descriptorSet) },
			pushConstantsTime
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, particleBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, trailBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, pointBuffer.getHandle());
		
		auto trailDispatchCount = vkcv::dispatchInvocations(trailBuffer.getCount(), 256);
		
		core.recordBeginDebugLabel(cmdStream, "Trail update", { 0.0f, 0.0f, 1.0f, 1.0f });
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			trailComputePipeline,
			trailDispatchCount,
			{
				vkcv::useDescriptorSet(0, descriptorSet),
				vkcv::useDescriptorSet(1, trailDescriptorSet)
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
			particlePipeline,
			pushConstantsDraw0,
			{ drawcallParticle },
			{ colorBuffers[0] },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		vkcv::PushConstants pushConstantsVoxel (sizeof(glm::mat4));
		pushConstantsVoxel.appendDrawcall(camera.getMVP());
		
		core.recordBeginDebugLabel(cmdStream, "Particle voxel update", { 1.0f, 0.5f, 0.8f, 1.0f });
		core.prepareImageForStorage(cmdStream, voxelRed.getHandle());
		core.prepareImageForStorage(cmdStream, voxelGreen.getHandle());
		core.prepareImageForStorage(cmdStream, voxelBlue.getHandle());
		core.prepareImageForStorage(cmdStream, voxelDensity.getHandle());
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			voxelParticlePipeline,
			particleDispatchCount,
			{
				vkcv::useDescriptorSet(0, descriptorSet),
				vkcv::useDescriptorSet(1, voxelDescriptorSet)
			},
			pushConstantsVoxel
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
			smokePipeline,
			pushConstantsDraw1,
			{ drawcallSmoke },
			{ colorBuffers[1] },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Smoke voxel update", { 1.0f, 0.7f, 0.8f, 1.0f });
		core.prepareImageForStorage(cmdStream, voxelRed.getHandle());
		core.prepareImageForStorage(cmdStream, voxelGreen.getHandle());
		core.prepareImageForStorage(cmdStream, voxelBlue.getHandle());
		core.prepareImageForStorage(cmdStream, voxelDensity.getHandle());
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			voxelSmokePipeline,
			smokeDispatchCount,
			{
				vkcv::useDescriptorSet(0, smokeDescriptorSet),
				vkcv::useDescriptorSet(1, voxelDescriptorSet)
			},
			pushConstantsVoxel
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBufferMemoryBarrier(cmdStream, trailBuffer.getHandle());
		core.recordBufferMemoryBarrier(cmdStream, pointBuffer.getHandle());
		
		core.recordBeginDebugLabel(cmdStream, "Draw trails", { 0.75f, 0.5f, 1.0f, 1.0f });
		core.recordDrawcallsToCmdStream(
			cmdStream,
			trailPipeline,
			pushConstantsDraw1,
			{ drawcallTrail },
			{ colorBuffers[2] },
			windowHandle
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Trail voxel update", { 1.0f, 0.9f, 0.8f, 1.0f });
		core.prepareImageForStorage(cmdStream, voxelRed.getHandle());
		core.prepareImageForStorage(cmdStream, voxelGreen.getHandle());
		core.prepareImageForStorage(cmdStream, voxelBlue.getHandle());
		core.prepareImageForStorage(cmdStream, voxelDensity.getHandle());
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			voxelTrailPipeline,
			trailDispatchCount,
			{
				vkcv::useDescriptorSet(0, trailDescriptorSet),
				vkcv::useDescriptorSet(1, voxelDescriptorSet)
			},
			pushConstantsVoxel
		);
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Combine voxel data", { 0.5f, 0.5f, 0.5f, 1.0f });
		
		core.prepareImageForStorage(cmdStream, voxelRed.getHandle());
		core.prepareImageForStorage(cmdStream, voxelGreen.getHandle());
		core.prepareImageForStorage(cmdStream, voxelBlue.getHandle());
		core.prepareImageForStorage(cmdStream, voxelDensity.getHandle());
		
		core.prepareImageForStorage(cmdStream, voxelData[0]);
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			voxelPipeline,
			voxelDispatchCount,
			{
				vkcv::useDescriptorSet(0, voxelDescriptorSet),
				vkcv::useDescriptorSet(1, voxelOutDescriptorSet)
			},
			vkcv::PushConstants(0)
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Fluid voxel data", { 0.2f, 0.2f, 0.9f, 1.0f });
		
		for (size_t i = 0; i < 8; i++) {
			core.prepareImageForSampling(cmdStream, voxelData[i % 2]);
			core.prepareImageForStorage(cmdStream, voxelData[(i + 1) % 2]);
		
			core.recordComputeDispatchToCmdStream(
				cmdStream,
				fluidPipeline,
				voxelDispatchCount,
				{ vkcv::useDescriptorSet(0, fluidDescriptorSet[i % 2]) },
				vkcv::PushConstants(0)
			);
		}
		
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Sample voxels", { 0.5f, 0.5f, 1.0f, 1.0f });
		
		core.prepareImageForStorage(cmdStream, voxelData[0]);
		core.prepareImageForStorage(cmdStream, voxelSamples.getHandle());
		
		auto sampleDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(voxelWidth, voxelHeight),
				vkcv::DispatchSize(8, 8)
		);
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			voxelSamplePipeline,
			sampleDispatchCount,
			{
				vkcv::useDescriptorSet(0, voxelOutDescriptorSet),
				vkcv::useDescriptorSet(1, samplesDescriptorSet)
			},
			vkcv::PushConstants(0)
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		core.recordBeginDebugLabel(cmdStream, "Add rendered images", { 0.5f, 0.5f, 1.0f, 1.0f });
		
		vkcv::DescriptorWrites addDescriptorWrites;
		addDescriptorWrites.writeSampledImage(0, voxelSamples.getHandle());
		addDescriptorWrites.writeSampler(1, voxelSampler);
		
		for (size_t i = 0; i < colorBuffers.size(); i++) {
			addDescriptorWrites.writeStorageImage(2 + i, colorBuffers[i]);
			core.prepareImageForStorage(cmdStream, colorBuffers[i]);
		}
		
		core.writeDescriptorSet(addDescriptor, addDescriptorWrites);
		core.prepareImageForSampling(cmdStream, voxelSamples.getHandle());
		
		auto colorDispatchCount = vkcv::dispatchInvocations(
				vkcv::DispatchSize(swapchainWidth, swapchainHeight),
				vkcv::DispatchSize(8, 8)
		);
		
		core.recordComputeDispatchToCmdStream(
			cmdStream,
			addPipe,
			colorDispatchCount,
			{
				vkcv::useDescriptorSet(0, addDescriptor),
				vkcv::useDescriptorSet(1, generationDescriptorSet)
			},
			vkcv::PushConstants(0)
		);
		
		core.recordEndDebugLabel(cmdStream);
		
		bloomAndFlares.recordEffect(cmdStream, colorBuffers.back(), colorBuffers.back());
		toneMapping.recordToneMapping(cmdStream, colorBuffers.back(), colorBuffers.back());
		gammaCorrection.recordEffect(cmdStream, colorBuffers.back(), swapchainImage);
		
		core.prepareSwapchainImageForPresent(cmdStream);
		core.submitCommandStream(cmdStream);
		
		gui.beginGUI();
		ImGui::Begin("Settings");
		
		bool firework, sparkler, nested;
		if (ImGui::BeginListBox(" ")) {
			firework = ImGui::Selectable("Firework");
			sparkler = ImGui::Selectable("Sparkler");
			nested = ImGui::Selectable("Nested Firework");
			
			ImGui::EndListBox();
		}
		
		bool resetTime = ImGui::Button("Reset");
		auto color = glm::vec3(0.0f);
		
		if (!events.empty()) {
			color = events[0].color;
		}
		
		bool colorChanged = ImGui::ColorPicker3("Color", (float*) & color);
		
		float gamma = gammaCorrection.getGamma();
		if (ImGui::SliderFloat("Gamma", &gamma, 0.0f, 10.0f)) {
			gammaCorrection.setGamma(gamma);
		}
		
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
